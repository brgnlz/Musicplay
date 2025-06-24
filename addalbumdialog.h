
#ifndef ADDALBUMDIALOG_H
#define ADDALBUMDIALOG_H

#include <QDialog>
#include <QPixmap>

namespace Ui {
class AddAlbumDialog;
}

class AddAlbumDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddAlbumDialog(QWidget *parent = nullptr);
    ~AddAlbumDialog();

    QString getTitle() const;
    QString getArtist() const;
    QString getGenre() const;
    int getYear() const;
    QPixmap getCover() const;

private slots:
    void on_loadCover_clicked();
    void on_saveButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::AddAlbumDialog *ui;
    QPixmap albumCover;
};

#endif 
