#ifndef PLAYERWINDOW_H
#define PLAYERWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMessageBox>
#include "track.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PlayerWindow; }
QT_END_NAMESPACE

class PlayerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PlayerWindow(QWidget *parent = nullptr);
    ~PlayerWindow();

    void setTrackList(const QList<Track> &tracks, int index); 
    void setTrack(const Track &track);                        

private slots:
    void onPlayClicked();
    void onPauseClicked();
    void onStopClicked();
    void updatePosition(qint64 position);
    void updateDuration(qint64 duration);
    void seek(int position);
    void on_btnNext_clicked();
    void on_btnPrev_clicked();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::PlayerWindow *ui;
    QMediaPlayer *mediaPlayer;
    QAudioOutput *audioOutput;

    Track currentTrack;
    QList<Track> trackList;   
    int currentIndex = -1;    
};

#endif 
