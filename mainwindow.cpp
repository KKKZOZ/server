#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    iniUI();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::iniUI(){
    currentStateLabel=new QLabel;
    currentStateLabel->setMinimumWidth(150);
    currentStateLabel->setText("未开始监听");
    ui->statusBar->addWidget(currentStateLabel);
}

void MainWindow::receiveNews(){
    QByteArray content = serverSocket -> readAll();
    QString text = QVariant(content).toString();
    //准备大规模重写
    if(text=="headReceiveSuccess"){
        sendFile();
        return;
    }

    if(isHead==false && isFile==false){
        if(text == "headiscoming"){
            isHead = true;
        }else{
            QString clientAddr = serverSocket->peerAddress().toString();
            ui ->textEdit->append(clientAddr + ": " + text);
        }
    }else if(isHead){
        isHead = false;
        isFile = true;
        qDebug()<<content;
        fileName = QVariant(content).toString().section("##",0,0);
        fileSize = QVariant(content).toString().section("##",1,1).toInt();
        //暂不设置progressbar
        recSize = 0;
        receiveFile.setFileName(fileName);
        receiveFile.open(QIODevice::WriteOnly);
        serverSocket->write("headReceiveSuccess");
    }else if(isFile){
        //QByteArray buf = textSocket -> readAll();
        qint64 len = receiveFile.write((content));
        recSize += len;
        //ui->progressBar->setValue(recSize);
        if(recSize == fileSize){
            receiveFile.close();
            QMessageBox::information(this,"完成","文件接收完毕");
            isFile = false;
        }
    }





    /*
    if(text != "headReceiveSuccess"){
        QString clientAddr = serverSocket->peerAddress().toString();
        ui ->textEdit->append(clientAddr + ": " + text);
    }else{
        sendFile();
    }
    */
}
void MainWindow::newConnect(){
    serverSocket = tcpServer ->nextPendingConnection();
    QString clientIP = serverSocket->peerAddress().toString();
    qDebug()<<clientIP;
    ui -> textEdit ->append(clientIP + "与服务器成功建立连接！");
    connect(serverSocket,SIGNAL(readyRead()),this,SLOT(receiveNews()));
    qDebug()<<"rew";
}

void MainWindow::startListen()
{
    currentStateLabel->setText("正在监听...");
    tcpServer = new QTcpServer();
    tcpServer->listen(QHostAddress::Any,9999);
    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(newConnect()));
    qDebug()<<"fdsj";
}

void MainWindow::on_pushButton_clicked()
{
    startListen();
}

void MainWindow::on_pushButton_2_clicked()
{
    QString text = ui->textEdit_2->toPlainText();
    serverSocket->write(text.toStdString().c_str(),strlen(text.toStdString().c_str()));
    ui -> textEdit_2->clear();
}

void MainWindow::on_pushButton_3_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,tr("Open File"),"../");
    if(!filePath.isEmpty()){
        fileName.clear();
        fileSize = 0;
        //获取文件名和大小
        QFileInfo info(filePath);
        fileName = info.fileName();
        fileSize = info.size();
        sendSize = 0;
        file.setFileName(filePath);
        file.open(QIODevice::ReadOnly);
        ui->lineEdit->setText(filePath);
        serverSocket->write("headiscoming");

    }else{
        ui->lineEdit->setText("选择文件路径无效");
    }
}
void MainWindow::on_pushButton_4_clicked()
{
    QString head = QString("%1##%2").arg(fileName).arg(fileSize);
    qint64 len = serverSocket->write(head.toUtf8());
    if(len < 0){
        QMessageBox::warning(this,"警告","文件头部发送失败！");
        file.close();
    }
}
void MainWindow::sendFile(){
    qint64 len = 0;
    do{
        char buf[4*1024];
        len = 0;
        len = file.read(buf,4*1024);
        len = serverSocket->write(buf,len);
    }while(len>0);
}

void MainWindow::on_actListen_triggered()
{
    startListen();
}
