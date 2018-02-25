#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>

#include <QUrl>

#include <vector>

class OpenDataFile;
class QMediaPlayer;
class QVideoWidget;
namespace AlenkaFile {
struct Event;
}

/**
 * @brief Implements position synchronized vide player.
 */
class VideoPlayer : public QWidget {
  Q_OBJECT

  struct VideoFile {
    int position, duration;
    double offset;
    QUrl url;
  };

  OpenDataFile *file = nullptr;
  std::vector<VideoFile> fileList;
  bool playing = false;
  QUrl currentVideoUrl;

  QVideoWidget *videoWidget;
  QMediaPlayer *player;
  std::vector<QMetaObject::Connection> connections;

public:
  explicit VideoPlayer(QWidget *parent = nullptr);

  void changeFile(OpenDataFile *file);

public slots:
  void togglePlay();

private slots:
  void selectMontage(int montageIndex);
  void updateFileList();
  void seek(int position);

private:
  void addVideoFile(const AlenkaFile::Event &event);
  std::pair<bool, VideoFile> selectFile(int position);
};

#endif // VIDEOPLAYER_H
