#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "albumswindow.h"
#include <QMainWindow>
#include "playlistswindow.h"




QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class AllTracksWindow;
class AlbumsWindow;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent *event) override;
private slots:
    void openAlbumsWindow();
     void on_btnAllTracks_clicked();
     void on_btnPlaylists_clicked();




private:
    AlbumsWindow *albumsWindow = nullptr;
    Ui::MainWindow *ui;
    QList<QJsonObject> albums;
    AllTracksWindow *allTracksWindow;  
    PlaylistsWindow *playlistsWindow;
    void loadAlbumsFromFile();
    void updateAlbumInList(const QJsonObject &updatedAlbum);




};

#endif 
