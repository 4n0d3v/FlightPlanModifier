#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QGeoCoordinate>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();
    bool error;

private slots:

    void on_b_browse_clicked();

    void on_l_file_textChanged(const QString &arg1);

    void on_b_update_clicked();

    void on_l_coords_textChanged(const QString &arg1);

    void on_l_subst_textChanged(const QString &arg1);

    void on_b_close_clicked();

    void on_b_help_clicked();

private:
    Ui::Dialog *ui;
    QGeoCoordinate coords;
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    QMap<QString, QString> opts;
};
#endif // DIALOG_H
