/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#ifndef MPRIS_H
#define MPRIS_H

#include "core/player.h"

#include <QDateTime>
#include <QDBusArgument>
#include <QObject>

class Player;
class Playlist;

struct DBusStatus {    // From Amarok.
  int play;            // Playing = 0, Paused = 1, Stopped = 2
  int random;          // Linearly = 0, Randomly = 1
  int repeat;          // Go_To_Next = 0, Repeat_Current = 1
  int repeat_playlist; // Stop_When_Finished = 0, Never_Give_Up_Playing = 1, Never_Let_You_Down = 42

  enum MprisPlayState {
    Mpris_Playing = 0,
    Mpris_Paused = 1,
    Mpris_Stopped = 2,
  };
};
Q_DECLARE_METATYPE(DBusStatus);

QDBusArgument& operator <<(QDBusArgument& arg, const DBusStatus& status);
const QDBusArgument& operator >>(const QDBusArgument& arg, DBusStatus& status);


struct Version {
  quint16 minor;
  quint16 major;
};
Q_DECLARE_METATYPE(Version);

QDBusArgument& operator <<(QDBusArgument& arg, const Version& version);
const QDBusArgument& operator >>(const QDBusArgument& arg, Version& version);


namespace mpris {

enum DBusCaps {
  NONE                 = 0,
  CAN_GO_NEXT          = 1 << 0,
  CAN_GO_PREV          = 1 << 1,
  CAN_PAUSE            = 1 << 2,
  CAN_PLAY             = 1 << 3,
  CAN_SEEK             = 1 << 4,
  CAN_PROVIDE_METADATA = 1 << 5,
  CAN_HAS_TRACKLIST    = 1 << 6,
};


class ArtLoader;
class Mpris1Root;
class Mpris1Player;
class Mpris1TrackList;


class Mpris1 : public QObject {
  Q_OBJECT

public:
  Mpris1(Player* player, ArtLoader* art_loader, QObject* parent = 0);

  static QVariantMap GetMetadata(const Song& song);

  Mpris1Root* root() const { return root_; }
  Mpris1Player* player() const { return player_; }
  Mpris1TrackList* tracklist() const { return tracklist_; }

private:
  Mpris1Root* root_;
  Mpris1Player* player_;
  Mpris1TrackList* tracklist_;
};


class Mpris1Root : public QObject {
  Q_OBJECT

public:
  Mpris1Root(Player* player, QObject* parent);

  QString Identity();
  void Quit();
  Version MprisVersion();

private:
  Player* player_;
};


class Mpris1Player : public QObject {
  Q_OBJECT

public:
  Mpris1Player(Player* player, QObject* parent);

  void Pause();
  void Stop();
  void Prev();
  void Play();
  void Next();
  void Repeat(bool);

  // those methods will use engine's state obtained with player->GetState() method
  DBusStatus GetStatus() const;
  int GetCaps() const;
  // those methods will use engine's state provided as an argument
  DBusStatus GetStatus(Engine::State state) const;
  int GetCaps(Engine::State state) const;

  void VolumeSet(int volume);
  int VolumeGet() const;
  void PositionSet(int pos);
  int PositionGet() const;
  QVariantMap GetMetadata() const;

  // Amarok extensions
  void VolumeUp(int vol);
  void VolumeDown(int vol);
  void Mute();
  void ShowOSD();

public slots:
  void CurrentSongChanged(const Song& song, const QString& art_uri);

signals:
  void CapsChange(int);
  void TrackChange(const QVariantMap&);
  void StatusChange(DBusStatus);

private slots:
  void PlaylistManagerInitialized();

  void EngineStateChanged(Engine::State state);
  void ShuffleModeChanged();
  void RepeatModeChanged();

private:
  Player* player_;

  QVariantMap last_metadata_;
};


class Mpris1TrackList : public QObject {
  Q_OBJECT

public:
  Mpris1TrackList(Player* player, QObject* parent);

  int AddTrack(const QString&, bool);
  void DelTrack(int index);
  int GetCurrentTrack() const;
  int GetLength() const;
  QVariantMap GetMetadata(int) const;
  void SetLoop(bool enable);
  void SetRandom(bool enable);

  // Amarok extension
  void PlayTrack(int index);

signals:
  void TrackListChange(int i);

private slots:
  void PlaylistChanged(Playlist* playlist);

private:
  Player* player_;
};

} // namespace mpris

#endif // MPRIS_H
