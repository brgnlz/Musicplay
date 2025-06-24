#ifndef PLAYLISTTRACKSWINDOW_H
#define PLAYLISTTRACKSWINDOW_H

#include <QWidget>
#include <QJsonObject>
#include <QStandardItemModel>
#include "playerwindow.h"
#include <QDialog>
#include <QAbstractItemView>





namespace Ui {
class PlaylistTracksWindow;
}

class PlaylistTracksWindow : public QDialog

{
    Q_OBJECT

public:
    explicit PlaylistTracksWindow(QWidget *parent = nullptr, const QList<QJsonObject> &albumsList = {});
    void setPlaylist(const QJsonObject &playlist);
    void addTracks(const QList<QJsonObject> &tracks);
    void setAlbums(const QList<QJsonObject> &albumList);
    ~PlaylistTracksWindow();
signals:
    void playlistUpdated(const QJsonObject &updatedPlaylist);
private slots:
    void onTrackDoubleClicked(const QModelIndex &index);
    void on_btnPlay_clicked();
    void on_btnDelete_clicked();
    void on_btnAddTrack_clicked();


private:
    Ui::PlaylistTracksWindow *ui;
    QStandardItemModel *model;
    PlayerWindow *playerWindow = nullptr;
    QList<QJsonObject> albums;

      QJsonObject playlist;
      void refreshUI();

    void loadTracks();
};

#endif 
