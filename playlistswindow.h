#ifndef PLAYLISTSWINDOW_H
#define PLAYLISTSWINDOW_H

#include <QWidget>
#include <QJsonObject>
#include <QJsonArray>

namespace Ui {
class PlaylistsWindow;
}

class PlaylistsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PlaylistsWindow(QWidget *parent = nullptr, const QList<QJsonObject> &albumsList = {});
     void setAlbums(const QList<QJsonObject> &albumsList);
    ~PlaylistsWindow();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;  

private slots:
    void on_btnAddPlaylist_clicked();
    void openPlaylistDetails(int index);
    void editSelectedPlaylist();
    void deleteSelectedPlaylist();

private:
    Ui::PlaylistsWindow *ui;
    QJsonArray playlists;
     QList<QJsonObject> albums;  

    void loadPlaylists();
    void savePlaylists();
    void refreshUI();

    int selectedPlaylistIndex = -1;  
};

#endif 
