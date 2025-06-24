#ifndef ALBUMSWINDOW_H
#define ALBUMSWINDOW_H

#include "albumtrackswindow.h"
#include "ui_albumswindow.h"
#include <QMainWindow>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QCloseEvent>
#include <QLineEdit>
#include <QComboBox>

namespace Ui {
class AlbumsWindow;
}

class AlbumsWindow : public QMainWindow
{
    Q_OBJECT
protected:
    void closeEvent(QCloseEvent *event) override;

signals:
    void windowClosed();
    void albumUpdated(const QJsonObject &updated);
    void requestUpdateAllTracks();  
    void albumsChanged(); 


public:
    explicit AlbumsWindow(QWidget *parent = nullptr);
    ~AlbumsWindow();
    void saveData();
    QList<QJsonObject> getAlbums() const;
    void setAlbums(const QList<QJsonObject> &list);


private slots:
    void editAlbum();
    void deleteAlbum();
    void on_addAlbumButton_clicked();
    void loadData();
    void onAlbumDoubleClicked(const QModelIndex &index);
    void openSelectedAlbum();
    void onOpenAlbumClicked();
    void onAlbumUpdated(const QJsonObject &updatedAlbum);



private:
    Ui::AlbumsWindow *ui;
    QStandardItemModel *model;
    QSortFilterProxyModel *proxyModel;
    void setupModel();
    QList<QJsonObject> albums;
     void playTrackFromAlbum(int row);
    QJsonObject currentAlbum;
     void refreshUI();

};

#endif 
