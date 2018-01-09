#include "formcreatenewfile.h"
#include "ui_formcreatenewfile.h"
#include <__common.h>
#include <QFileDialog>

formCreateNewFile::formCreateNewFile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::formCreateNewFile)
{
    ui->setupUi(this);
}

formCreateNewFile::~formCreateNewFile()
{
    delete ui;
}

void formCreateNewFile::on_pbClose_clicked()
{
    this->close();
}

void formCreateNewFile::on_pbSave_clicked()
{
    //Chech Filename not Empty
    if( ui->txtFilename->text().trimmed().isEmpty() || ui->txtFilename->text().trimmed() == "./" )
    {
        funcShowMsgERROR("Filename is Empty!");
        ui->txtFilename->setFocus();
        return (void)false;
    }

    //Chech File-contain not Empty
    if( ui->txtContain->toPlainText().trimmed().isEmpty() )
    {
        funcShowMsgERROR("File-Contain is Empty!");
        ui->txtContain->setFocus();
        return (void)false;
    }

    //Create New File
    QString filePath, fileContain;
    filePath = ui->txtFilename->text().trimmed();
    fileContain = ui->txtContain->toPlainText();
    saveFile(filePath,fileContain);
    if( !fileExists(filePath) )
    {
        funcShowMsgERROR("Sorry, File not Created");
    }
    else
    {
        funcShowMsg("SUCCESS","File Created Successfully!");
    }

}

void formCreateNewFile::on_pbOpen_clicked()
{
    this->setVisible(false);


    QString filePath = QFileDialog::getOpenFileName(
                                                        this,
                                                        tr("Select File..."),
                                                        "./",
                                                        "(*.*);;"
                                                     );
    if( !filePath.isEmpty() ){
        ui->txtFilename->setText(filePath);

        QString Contain;
        Contain = readAllFile(filePath);
        ui->txtContain->setPlainText(Contain);
    }

    this->setVisible(true);
}
