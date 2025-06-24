#ifndef ALBUMTRACKSWINDOW_H
#define ALBUMTRACKSWINDOW_H
#include "playerwindow.h"
#include "track.h"
#include <QWidget>
#include <QJsonObject>
#include <QList>
#include <QStandardItemModel>





namespace Ui {
class AlbumTracksWindow;
}

class AlbumTracksWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AlbumTracksWindow(const QJsonObject& album, QWidget *parent = nullptr);
    ~AlbumTracksWindow();
signals:
    void albumUpdated(const QJsonObject &album);
    void requestUpdateAllTracks(); 
private slots:
    void onAddTrack();
    void onEditTrack();
    void onDeleteTrack();
    void onPlayTrack();
protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::AlbumTracksWindow *ui;
    QJsonObject albumData;
    QList<Track> trackList;
    QStandardItemModel* model;
    PlayerWindow *playerWindow = nullptr;

    int currentAlbumId; 





    void loadAlbumTracks();
    void saveAlbumTracks();
    void updateTable();
};

#endif 
