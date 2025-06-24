#ifndef PLAYLISTEDITDIALOG_H
#define PLAYLISTEDITDIALOG_H

#include <QDialog>
#include <QJsonObject>

namespace Ui {
class PlaylistEditDialog;
}

class PlaylistEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlaylistEditDialog(QWidget *parent = nullptr);
    void setData(const QJsonObject &playlist);
    QJsonObject getData() const;

private slots:
    void on_btnSelectCover_clicked();
    void on_btnOk_clicked();
    void on_btnCancel_clicked();

private:
    Ui::PlaylistEditDialog *ui;
    QString coverPath;
};

#endif 
