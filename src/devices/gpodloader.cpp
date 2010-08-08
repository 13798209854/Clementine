/* This file is part of Clementine.

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gpodloader.h"
#include "core/song.h"
#include "core/taskmanager.h"
#include "library/librarybackend.h"

#include <gpod/itdb.h>

#include <QtDebug>

GPodLoader::GPodLoader(const QString& mount_point, TaskManager* task_manager,
                       LibraryBackend* backend, QObject *parent)
  : QObject(parent),
    mount_point_(mount_point),
    type_(Song::Type_Unknown),
    task_manager_(task_manager),
    backend_(backend)
{
  original_thread_ = thread();
}

void GPodLoader::LoadDatabase() {
  int task_id = task_manager_->StartTask(tr("Loading iPod database"));
  emit TaskStarted(task_id);

  // Load the iTunes database
  GError* error = NULL;
  Itdb_iTunesDB* db = itdb_parse(mount_point_.toLocal8Bit(), &error);

  // Check for errors
  if (!db) {
    qDebug() << "GPodLoader error:" << error->message;
    emit Error(QString::fromUtf8(error->message));
    g_error_free(error);
    task_manager_->SetTaskFinished(task_id);
    return;
  }

  // Convert all the tracks from libgpod structs into Song classes
  SongList songs;
  for (GList* tracks = db->tracks ; tracks != NULL ; tracks = tracks->next) {
    Itdb_Track* track = static_cast<Itdb_Track*>(tracks->data);

    Song song;
    song.InitFromItdb(track);
    song.set_directory_id(1);
    song.set_filename((path_prefix_.isEmpty() ? mount_point_ : path_prefix_) +
                      song.filename());
    if (type_ != Song::Type_Unknown)
      song.set_filetype(type_);
    songs << song;
  }

  // Need to remove all the existing songs in the database first
  backend_->DeleteSongs(backend_->FindSongsInDirectory(1));

  // Add the songs we've just loaded
  backend_->AddOrUpdateSongs(songs);

  moveToThread(original_thread_);

  task_manager_->SetTaskFinished(task_id);
  emit LoadFinished(db);
}
