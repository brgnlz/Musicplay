#ifndef ALLTRACKSWINDOW_H
#define ALLTRACKSWINDOW_H
#include "track.h"
#include "playerwindow.h"
#include <QWidget>
#include <QStandardItemModel>
#include <QJsonObject>
#include <QSortFilterProxyModel>
#include <QDialog>


namespace Ui {
class AllTracksWindow;
}

class AllTracksWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AllTracksWindow(QWidget *parent = nullptr);
    ~AllTracksWindow();

    void loadTracksFromAlbums(const QList<QJsonObject> &albums); 
signals:
    void tracksSelected(const QList<QJsonObject> &selectedTracks);
    void tracksSelectedForPlaylist(const QList<QJsonObject> &tracks);



private slots:
    void playTrack(const Track &track);
     void onTrackDoubleClicked(const QModelIndex &index);
     void onAddToPlaylistClicked();
private:
    Ui::AllTracksWindow *ui;
    QStandardItemModel *model;
    QSortFilterProxyModel *proxyModel;
    PlayerWindow *playerWindow = nullptr;
};

#endif 
