#include "dialog.h"
#include "ui_dialog.h"
#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileDialog>
#include <QTextStream>
#include <QTextCodec>
#include <QMessageBox>


QString getOkCoord (QStringList &co, int index, bool mode2=false){
    QString curCoord = co.at(index).trimmed();
    QString okCoord = curCoord.at(curCoord.length()-1);
    okCoord += curCoord.left(curCoord.length()-1);
    if (!mode2)
        return okCoord;
    int pos = okCoord.lastIndexOf(".");
    if (pos < 0)
        pos = okCoord.length()-1;
    okCoord = okCoord.left(pos);
    return okCoord.replace("' ", ".") + "'";
}


Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    QFile f (qApp->applicationDirPath()+"/opt.txt");
    if (!f.exists())
    {
        if (f.open(QIODevice::WriteOnly|QIODevice::Text)){
            QTextStream out (&f);
            out << "WayPointStyleName=Intersection";
            out.flush();
            f.close();
        }
    }
    if (f.open(QIODevice::ReadOnly|QIODevice::Text)){
        QTextStream in (&f);
        QString tmp;
        QStringList opt;
        while (in.readLineInto(&tmp))
            opt.append(tmp);
        for (QString s : opt){
            int pos = s.indexOf("=");
            opts.insert(s.left(pos),s.right(s.length()-pos-1));
        }
    }
    error = false;
    if (!opts.contains("WayPointStyleName")){
        QMessageBox::critical(this,"I'm just a tool","This tool must be allowed to write in its own folder.\ni.e. Don't install into your programfiles but just paste its folder on your desktop or docs or so...");
        error = true;
    }
    setWindowFlags(Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void Dialog::dropEvent(QDropEvent *e)
{
    if (e->mimeData()->urls().count() > 0){
        QString fileName = e->mimeData()->urls().at(0).toLocalFile();
        if (fileName.endsWith("pln",Qt::CaseInsensitive) || fileName.endsWith("flt",Qt::CaseInsensitive))
            ui->l_file->setText(fileName);
        else
            QMessageBox::warning(this,"error","this field only accepts .PLN and .FLT files");
    }
}

void Dialog::on_b_browse_clicked()
{
    ui->l_file->setText(QFileDialog::getOpenFileName(this,"choose a pln or flt file", QString(), "*.pln;*.flt"));
}




void Dialog::on_l_file_textChanged(const QString &arg1)
{
    ui->l_waypoints->clear();
    QString file = arg1.endsWith("pln",Qt::CaseInsensitive) ? arg1 : arg1.left (arg1.length()-3) + "pln";
    //treat pln file
    QFile f (file);
    if (!f.open(QIODevice::ReadOnly|QIODevice::Text))
        return;
    QTextStream in(&f);
    QString previousLine, currentLine, keyword;
    while (in.readLineInto(&currentLine)){
        if (currentLine.contains(opts.value("WayPointStyleName"))){
            int pos = previousLine.indexOf("id=")+4;
            keyword = previousLine.left (previousLine.length()-2);
            keyword = keyword.right(keyword.length() - pos);
            ui->l_waypoints->addItem(keyword);
        }
        previousLine = currentLine;
    }
}

void Dialog::on_b_update_clicked()
{
    QString arg1 = ui->l_file->text();
    QString file = arg1.endsWith("pln",Qt::CaseInsensitive) ? arg1 : arg1.left (arg1.length()-3) + "pln";
    //treat pln file
    QFile f (file);
    QString subst = ui->l_subst->text();
    if ((ui->l_waypoints->currentItem() == nullptr) || (!f.open(QIODevice::ReadOnly|QIODevice::Text)) || subst.isEmpty()){
        QMessageBox::warning(this,"no selection","Choose a waypoint to substitute and give a text for the substitution");
        return;
    }
    QString keyword = ui->l_waypoints->currentItem()->text();
    QTextStream in(&f);
    in.setCodec("UTF-8");
    QString wholeContent = in.readAll();
    f.close();
    wholeContent.replace("\"" + keyword + "\"","\"" + subst + "\"");
    wholeContent.replace(">" + keyword + "<",">" + subst + "<");
    int pos = wholeContent.indexOf("\"" + subst + "\"");
    pos = wholeContent.indexOf("<WorldPosition>", pos);
    int pos2 = wholeContent.indexOf(",+",pos);
    QStringList okCoords = coords.toString().split(",");
    QString okCoord = getOkCoord(okCoords,0);
    okCoord += "," + getOkCoord(okCoords,1);
    QString newContent = wholeContent.left (pos+15) + okCoord + wholeContent.right (wholeContent.length()-pos2);
    if (f.open(QIODevice::WriteOnly|QIODevice::Text)){
        QTextStream out (&f);
        out.setCodec("UTF-8");
        out << newContent.replace("\r","");
        out.flush();
        f.close();
        QFile f2 (file.left(file.length()-3)+"flt");
        if (f2.open(QIODevice::ReadOnly|QIODevice::Text)){
            QTextStream in2 (&f2);
            in2.setCodec("ANSI");
            QStringList cntList;
            QString orgLine,newLine;
            while (in2.readLineInto(&newContent)){
                if (newContent.startsWith("Waypoint",Qt::CaseInsensitive) && newContent.contains(", "+keyword+",")){
                    QStringList wptDet = newContent.split(",");
                    wptDet[1] = " " + subst;
                    wptDet[3] = " " + subst;
                    wptDet[5] = " " + getOkCoord(okCoords,0,true);
                    wptDet[6] = " " + getOkCoord(okCoords,1,true);
                    cntList.append(wptDet.join(","));
                }else
                    cntList.append(newContent);
            }
            f2.close();
            if (f2.open(QIODevice::WriteOnly|QIODevice::Text)){
                QTextStream out2 (&f2);
                out2.setCodec("ANSI");
                for (QString s : cntList)
                    out2 << s << endl;
                out2.flush();
                f2.close();
                QMessageBox::information(this,"success","Files updated");
                on_l_file_textChanged(ui->l_file->text());
                ui->l_subst->clear();
                ui->l_coords->clear();
                return;
            }
        }
    }
    QMessageBox::critical(this,"error","Not able to work with the given files.. Permission issue ?");
}

void Dialog::on_l_coords_textChanged(const QString &arg1)
{
    if (arg1.isEmpty())
        return;
    QStringList qsl = arg1.split (",");
    coords.setLatitude(qsl.at(0).toDouble());
    coords.setLongitude(qsl.at(1).trimmed().toDouble());
}

void Dialog::on_l_subst_textChanged(const QString &arg1)
{
    ui->l_subst->setText(arg1.toUpper());
}

void Dialog::on_b_close_clicked()
{
    accept();
}

void Dialog::on_b_help_clicked()
{
    QString msg = "Step (1) Give the .PLN or the .FLT file to the program (by clicking the browse button or by dropping it anywhere in the program window from a file explorer).";
    msg += "\nStep (2) Select the waypoint you want to modify\nStep (3) Specify the new name for the selected waypoint\nStep (4) Paste or drop GPS coordinates in decimal format in ";
    msg += "the appropriate field\nStep (5) Click the Update button to write changes to the file\n\nRepeat steps 2 to 5 for each waypoint that you want to modify\n\n";
    msg += "NB After first launch an opt.txt file is created, the program only shows waypoint of the type specified in opt.txt (by default \"Intersection\")";
    QMessageBox::information(this,"help",msg);
}
