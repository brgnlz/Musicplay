#include "playerwindow.h"
#include "ui_playerwindow.h"
#include <QFile>
#include <QSlider>
#include <QCloseEvent>
#include <QTime>

PlayerWindow::PlayerWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PlayerWindow)
    , mediaPlayer(new QMediaPlayer(this))
    , audioOutput(new QAudioOutput(this))
{
    ui->setupUi(this);


    
    setWindowFlag(Qt::Window);
    setWindowTitle("Плеер");
    setMinimumSize(400, 300);

    
    mediaPlayer->setAudioOutput(audioOutput);

    
    connect(ui->btnPlay, &QPushButton::clicked, this, &PlayerWindow::onPlayClicked);
    connect(ui->btnPause, &QPushButton::clicked, this, &PlayerWindow::onPauseClicked);
    connect(ui->btnStop, &QPushButton::clicked, this, &PlayerWindow::onStopClicked);
    connect(ui->btnNext, &QPushButton::clicked, this, &PlayerWindow::on_btnNext_clicked);
    connect(ui->btnPrev, &QPushButton::clicked, this, &PlayerWindow::on_btnPrev_clicked);
    connect(ui->sliderProgress, &QSlider::sliderMoved, this, &PlayerWindow::seek);
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &PlayerWindow::updatePosition);
    connect(mediaPlayer, &QMediaPlayer::durationChanged, this, &PlayerWindow::updateDuration);
}



PlayerWindow::~PlayerWindow()
{
    delete ui;
}

void PlayerWindow::setTrack(const Track &track)
{
    currentTrack = track;

    if (!QFile::exists(track.filePath)) {
        QMessageBox::critical(this, "\u041e\u0448\u0438\u0431\u043a\u0430", "\u0424\u0430\u0439\u043b \u043d\u0435 \u043d\u0430\u0439\u0434\u0435\u043d:\n" + track.filePath);
        return;
    }

    mediaPlayer->setSource(QUrl::fromLocalFile(track.filePath));

    QString fixedTitle = track.title;
    fixedTitle.replace("&#039;", "'");
    ui->labelTitle->setText(QString::fromUtf8("🎵 ") + fixedTitle);  

    ui->labelArtist->setText(track.artist);
    ui->labelCover->clear();

    mediaPlayer->play();
}

void PlayerWindow::setTrackList(const QList<Track> &tracks, int index)
{
    if (tracks.isEmpty() || index < 0 || index >= tracks.size()) return;

    trackList = tracks;
    currentIndex = index;

    setTrack(trackList[currentIndex]);
}

void PlayerWindow::on_btnNext_clicked()
{
    if (currentIndex + 1 < trackList.size()) {
        currentIndex++;
        setTrack(trackList[currentIndex]);
    }
}

void PlayerWindow::on_btnPrev_clicked()
{
    if (currentIndex > 0) {
        currentIndex--;
        setTrack(trackList[currentIndex]);
    }
}

void PlayerWindow::onPlayClicked()
{
    if (currentTrack.filePath.isEmpty() || !QFile::exists(currentTrack.filePath)) {
        QMessageBox::warning(this, "\u041e\u0448\u0438\u0431\u043a\u0430", "\u0424\u0430\u0439\u043b \u0442\u0440\u0435\u043a\u0430 \u043d\u0435 \u0437\u0430\u0433\u0440\u0443\u0436\u0435\u043d \u0438\u043b\u0438 \u043f\u043e\u0432\u0440\u0435\u0436\u0434\u0451\u043d.");
        return;
    }
    mediaPlayer->play();
}

void PlayerWindow::onPauseClicked()
{
    if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState)
        mediaPlayer->pause();
}

void PlayerWindow::onStopClicked()
{
    mediaPlayer->stop();
    ui->sliderProgress->setValue(0);
    ui->labelCurrentTime->setText("0:00");
}

void PlayerWindow::updatePosition(qint64 position)
{
    ui->sliderProgress->setValue(position);

    QTime time(0, 0);
    time = time.addMSecs(position);
    ui->labelCurrentTime->setText(time.toString("m:ss"));
}

void PlayerWindow::updateDuration(qint64 duration)
{
    ui->sliderProgress->setRange(0, duration);

    QTime time(0, 0);
    time = time.addMSecs(duration);
    ui->labelDuration->setText(time.toString("m:ss"));
}

void PlayerWindow::seek(int position)
{
    mediaPlayer->setPosition(position);
}

void PlayerWindow::closeEvent(QCloseEvent *event)
{
    if (mediaPlayer && mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        mediaPlayer->stop();
    }
    QMainWindow::closeEvent(event);
}
