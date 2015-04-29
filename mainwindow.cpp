﻿#include <weh.h>
#include "mainwindow.h"

QSplitter *ras[3] ;
CodeWindow *codeedit ;
OtherWindow *otheredit ;
ProjectWindow *projectedit ;
BkeLeftFileWidget *fileListWidget ;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    isConnetct = false ;

    //this->setWindowFlags(Qt::FramelessWindowHint); //隐藏标题栏
    setWindowTitle("BKE Creator");
    codeedit = new CodeWindow(this) ;
    otheredit = new OtherWindow(this) ;
    projectedit = new ProjectWindow(this) ;
    projectedit->hide();
    fileListWidget = new BkeLeftFileWidget(this) ;
    fileListWidget->hide();

    CreateDownBar();

    for( int i = 0 ; i < 3 ; i++){
        ras[i] = new QSplitter(this) ;
        ras[i]->setHandleWidth(0);
    }

    //读取设置大小
    int xs,ys ;
    xs = BKE_CLOSE_SETTING->value("window/width").toInt() ;
    ys = BKE_CLOSE_SETTING->value("window/height").toInt() ;
    if( xs > 0 && ys > 0) resize(xs,ys);

    //项目树与文件框
    ras[0]->setOrientation(Qt::Vertical);
    //btnbar->hide();
    ras[0]->addWidget( btnbar ) ;

    int leftcount = BKE_CLOSE_SETTING->value("window/leftcount").toInt() ;
    if( leftcount < 1){
        OtherBasicWin *oth1 = new OtherBasicWin ;
        oth1->ChangeShow(" 项目");
        OtherBasicWin *oth2 = new OtherBasicWin ;
        oth2->ChangeShow(" 打开文档");
        ras[0]->addWidget(oth1);
        ras[0]->addWidget(oth2);
        ras[0]->setStretchFactor(0,2);
        ras[0]->setStretchFactor(1,1);
    }
    else{
        //读取左侧窗口设置
        QStringList ls = BKE_CLOSE_SETTING->value("window/leftname").toStringList() ;
        for( int i = 0 ; i < leftcount ; i++){
            OtherBasicWin *le = new OtherBasicWin ;
            le->ChangeShow( ls.at(i) );
            ras[0]->addWidget(le);
        }
    }

    //代码框与问题框
    ras[1]->setOrientation(Qt::Vertical);
    ras[1]->addWidget(codeedit->toolbar);
    ras[1]->addWidget(codeedit);
    ras[1]->addWidget(otheredit);
    otheredit->hide();

    ras[1]->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    ras[2]->setOrientation(Qt::Horizontal);
    ras[2]->addWidget(ras[0]);
    ras[2]->addWidget(ras[1]);
    ras[2]->setStretchFactor(0,1);
    ras[2]->setStretchFactor(1,2);

    this->setCentralWidget(ras[2]);
//    ras[2]->setHandleWidth(1);
    ras[2]->setStyleSheet("QSplitter::handle { background-color:#303030 }");

    //恢复位置
    for( int i = 0 ; i < 3 ; i++){
        QByteArray st = BKE_CLOSE_SETTING->value( QString("window/leftpos%1").arg(i) ).toByteArray() ;
        if( !st.isEmpty() ) ras[i]->restoreState(st) ;
    }

    this->addToolBar(Qt::BottomToolBarArea,downbar);

    CreateMenu();

    //绑定其他窗口，并连接信号
    codeedit->OtherWinOtherwin(otheredit);
    codeedit->OtherWinProject(projectedit);
    codeedit->OtherwinFileList(fileListWidget);
    //改变标题
    connect(codeedit,SIGNAL(CurrentFileChange(QString,QString)),this,SLOT(CurrentFileChange(QString,QString))) ;

    //注册事件过滤器
    QMainWindow::installEventFilter(this) ;

    _instance = this;
}

MainWindow *MainWindow::_instance = NULL;

MainWindow::~MainWindow()
{
    _instance = NULL;
}

void MainWindow::CreateMenu()
{
    QFont tff = this->menuBar()->font() ;
    tff.setPointSize(11);
    this->menuBar()->setFont(tff);
    this->menuBar()->setStyleSheet(BKE_SKIN_SETTING->value(BKE_SKIN_CURRENT+"/menubar",QString()).toString());

	btnnewprojectact = new QAction(QIcon(":/project/source/newproject.png"), "新建项目", this);
	btnopenprojectact = new QAction(QIcon(":/project/source/open_file.png"), "打开项目", this);
	btnopenfileact = new QAction(QIcon(":/project/source/openfile.png"), "打开文件", this);
	btnnewfileact = new QAction(QIcon(":/project/source/newfile.png"), "新建脚本", this);

    wmenu = this->menuBar()->addMenu("&文件");
    wmenu->setStyleSheet(BKE_SKIN_SETTING->value(BKE_SKIN_CURRENT+"/menu").toString());

    wmenu->addAction(btnopenprojectact) ;
    wmenu->addAction(btnnewprojectact) ;
    wmenu->addAction(btnopenfileact) ;
    wmenu->addAction(btnnewfileact) ;
    wmenu->addSeparator() ;
    rfmenu = wmenu->addMenu("最近访问的文件") ;
    connect(rfmenu,SIGNAL(aboutToShow()),this,SLOT(ReflashMenu())) ;
    rpmenu = wmenu->addMenu("最近使用的项目") ;
    connect(rpmenu,SIGNAL(aboutToShow()),this,SLOT(ReflashMenu())) ;
    wmenu->addSeparator() ;
    connect(wmenu->addAction("保存所有文件"),SIGNAL(triggered()),codeedit,SLOT(SaveALL())) ;
    connect(wmenu->addAction("关闭所有文件"),SIGNAL(triggered()),codeedit,SLOT(CloseAll())) ;
    connect(wmenu->addAction("退出"),SIGNAL(triggered()),this,SLOT(close())) ;

    wmenu = this->menuBar()->addMenu("&编辑");
    wmenu->setStyleSheet(BKE_SKIN_SETTING->value(BKE_SKIN_CURRENT+"/menu").toString());
    wmenu->addAction(codeedit->btnundoact) ;
    wmenu->addAction(codeedit->btnredoact) ;
    wmenu->addSeparator() ;
    wmenu->addAction(codeedit->btncutact) ;
    wmenu->addAction(codeedit->btncopyact) ;
    wmenu->addAction(codeedit->btnpasteact) ;
    wmenu->addAction(codeedit->btnselectall) ;
    wmenu->addSeparator() ;
    wmenu->addMenu( "高级" ) ;
    wmenu->addSeparator() ;
    wmenu->addAction(codeedit->btnfindact) ;
    wmenu->addAction(codeedit->btnreplaceact) ;
    wmenu->addAction("在文件中查找/替换...") ;
    wmenu->addAction(codeedit->btnfly) ;
    wmenu->addAction(codeedit->btncodeact) ;

    wmenu = this->menuBar()->addMenu("&构建");
    wmenu->setStyleSheet(BKE_SKIN_SETTING->value(BKE_SKIN_CURRENT+"/menu").toString());
    wmenu->addAction(codeedit->btncompileact ) ;
    wmenu->addAction(codeedit->btncompilerunact ) ;
    wmenu->addAction(codeedit->btnrunact ) ;

    wmenu = this->menuBar()->addMenu("&工具");
    wmenu->setStyleSheet(BKE_SKIN_SETTING->value(BKE_SKIN_CURRENT+"/menu").toString());
    connect(wmenu->addAction("选项..."),SIGNAL(triggered()),this,SLOT(Config())) ;
	btnReleaseGame = wmenu->addAction("发布游戏");
	connect(btnReleaseGame, SIGNAL(triggered()), this, SLOT(ReleaseGame()));
    wmenu = this->menuBar()->addMenu("&帮助");
    wmenu->setStyleSheet(BKE_SKIN_SETTING->value(BKE_SKIN_CURRENT+"/menu").toString());
    wmenu->addAction("帮助文件") ;
    connect(wmenu->addAction("检查更新"),SIGNAL(triggered()),this,SLOT(startUp())) ;
    connect(wmenu->addAction("(开启/关闭)自动更新"),SIGNAL(triggered()),this,SLOT(OCupdate())) ;
    connect(wmenu->addAction("Creator教程"),SIGNAL(triggered()),this,SLOT(HelpCreator())) ;
    connect(wmenu->addAction("关于..."),SIGNAL(triggered()),this,SLOT(AboutBkeCreator())) ;

    connect(btnnewprojectact,SIGNAL(triggered()),projectedit,SLOT(NewProject())) ;
    connect(btnopenprojectact,SIGNAL(triggered()),projectedit,SLOT(OpenProject())) ;
    connect(btnopenfileact,SIGNAL(triggered()),projectedit ,SLOT(OpenFile())) ;
    connect(btnnewfileact,SIGNAL(triggered()),codeedit,SLOT(NewEmptyFile())) ;

}

//创建下边栏
void MainWindow::CreateDownBar()
{
    downbar = new QToolBar(this) ;  //下边栏
    downbar->setFixedHeight(24);
    downbar->setMovable(false);

    //下边栏按钮
    btnhiddenleftact = new QAction(QIcon(":/left/source/hiddenleft.png"),"隐藏侧边栏",this) ;
    connect(btnhiddenleftact,SIGNAL(triggered()),this,SLOT(HiddenLeft())) ;
    downbar->addAction(btnhiddenleftact) ;


    QWidget *temp11 = new QWidget(this) ;
    temp11->setFixedWidth(240);
    downbar->addWidget(temp11) ;

    downbar->addWidget(otheredit->btnproblem) ;
    downbar->addWidget(otheredit->btnsearch) ;
    downbar->addWidget(otheredit->btncompiletext) ;
    downbar->addWidget(otheredit->btnbookmark) ;
    downbar->addWidget(otheredit->btnmark) ;

    btnbar = new QToolBar(this) ;
    btnbar->setFixedHeight(24);
    btnbar->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    //btnbar->setStyleSheet(BKE_QCSS_OBJECT.value("toolbar2").toString());
    btnbar->setStyleSheet(BKE_SKIN_SETTING->value(BKE_SKIN_CURRENT+"/codetoolbar").toString());

    editsearch = new QSearchLineEdit("快速搜索",this) ;
    btnbar->addWidget(editsearch) ;
}


void MainWindow::test()
{
}

void MainWindow::HiddenLeft()
{
    if( ras[0]->isHidden() ) ras[0]->show();
    else ras[0]->hide();
}

void MainWindow::AboutBkeCreator()
{
    LableSureDialog msg ;
    QString temp ;
    temp.append("  BKE Creator           \r\n\r\n") ;
    temp.append("  版本："+BKE_CREATOR_VERTION+"    \r\n") ;
    temp.append("  开发：萝莉岛&歪鼻子&Taigacon&gjy_管\r\n\r\n") ;
    temp.append("  BKE Creator处于完善开发阶段，有些功能无效是正常的。\r\n"
                "") ;
    msg.SetLable(temp);
    msg.SetBtn(QStringList()<<"查看源代码(SVN)"<<"确认(OK)");
    msg.SetDefaultBtn(1);
    if( msg.WaitUser() == 0 ) QDesktopServices::openUrl( QUrl("http://pan.baidu.com/s/1kTG9AvX"));
}


void MainWindow::closeEvent(QCloseEvent *e)
{
    bool res = codeedit->CloseAll();
	if (!res)
	{
		e->ignore();
		return;
	}
    codeedit->ClearCompile();

    BKE_CLOSE_SETTING->setValue("window/width",width());
    BKE_CLOSE_SETTING->setValue("window/height",height());

	QStringList ls;
    for( int i = 1 ; i < ras[0]->count() ; i++){
        OtherBasicWin *le = dynamic_cast<OtherBasicWin*>(ras[0]->widget(i)) ;
        if( le->isHidden() ) continue ;
        ls.append( le->winlist->currentText() );
    }
    BKE_CLOSE_SETTING->setValue("window/leftcount",ls.size());
    BKE_CLOSE_SETTING->setValue("window/leftname",ls);
    BKE_CLOSE_SETTING->setValue("window/leftpos0",ras[0]->saveState());
    BKE_CLOSE_SETTING->setValue("window/leftpos1",ras[1]->saveState());
    BKE_CLOSE_SETTING->setValue("window/leftpos2",ras[2]->saveState());
    BKE_CLOSE_SETTING->setValue("window/ismax",isMaximized());
}

QList<QAction*> MainWindow::SetMenuList(QMenu *mn,const QStringList &list)
{
    QList<QAction*> ls ;
    mn->clear();

    QAction *p ;
    for( int i = 0 ; i < list.size() && i< 10; i++){
        if( list.at(i).isEmpty() ) continue ;
        p = mn->addAction(list.at(i)) ;
        ls.append( p );
        connect(p,SIGNAL(triggered()),this,SLOT(Recentfile())) ;
    }

    ls.append( mn->addAction("清除记录") );
    connect(ls.at(ls.size()-1),SIGNAL(triggered()),this,SLOT(ClearMenu())) ;
    return ls ;
}

void MainWindow::ClearMenu()
{
    QAction *p = dynamic_cast<QAction*>(sender()) ;
    if( p == 0 ) return ;
    QMenu *mn = dynamic_cast<QMenu*>( p->parentWidget()) ;
    if( mn == 0 ) return ;
    else if( mn == rpmenu ) BkeCreator::AddRecentProject("##") ;
    else if( mn == rfmenu ) BkeCreator::AddRecentFile("##") ;
}

void MainWindow::ReflashMenu()
{
    QMenu *mn = dynamic_cast<QMenu*>(sender()) ;
    if( mn == NULL ) return ;
    else if( mn == rfmenu ) SetMenuList(mn,BKE_Recently_Files) ;
    else if( mn == rpmenu ) SetMenuList(mn,BKE_Recently_Project) ;
}

void MainWindow::Recentfile()
{
    QAction *p = dynamic_cast<QAction*>(sender()) ;
    if( p == NULL ) return ;
    else if( p->text().endsWith(".bkp") ) projectedit->OpenProject(p->text());
    else if( p->text().endsWith("bkscr")) codeedit->addFile(p->text(),"");
}

void MainWindow::NormalAction()
{
    QAction *p = dynamic_cast<QAction*>(sender()) ;
    if( p == NULL ) return ;
}


void MainWindow::Bedestroy()
{
}

void MainWindow::CurrentFileChange(const QString &name,const QString &prodir)
{
    QString title ;
    if( prodir.isEmpty() ) title = "["+name+"] - BKE Creator" ;
    else title = projectedit->workpro->ProjectName() +" ["+name+"] - BKE Creator" ;
    setWindowTitle(title);
}


void MainWindow::HelpCreator()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(BKE_CURRENT_DIR+"/tool/简介.html")) ;
}


//检查更新
void MainWindow::CheckUpdate()
{
    QString platform;

#ifdef Q_OS_WIN
    platform = "win/";
#endif
#ifdef Q_OS_LINUX
    platform = "";
#endif
#ifdef Q_OS_MAC
    platform = "mac/";
#endif
    netAdimin = new QNetworkAccessManager(this) ;
    connect(netAdimin,SIGNAL(finished(QNetworkReply*)),this,SLOT(upfileFinish(QNetworkReply*))) ;
    isConnetct = true ;
    netAdimin->get(QNetworkRequest(QUrl("http://bke.bakery.moe/update/"+platform+"bkecreator"))) ;
}

//检查升级数据包
void MainWindow::upfileFinish(QNetworkReply *netf)
{
    isConnetct = false ;
    QJsonDocument dc = QJsonDocument::fromJson(netf->readAll()) ;
    if( !dc.isEmpty() ){
        QJsonObject llm = dc.object() ;
        if( hasFileUp( llm.value("files").toObject()) ) isUpdate( llm );
    }

    netf->deleteLater();
}

//检查是否还在连接，如果是，终止，需要？
void MainWindow::CheckConnect()
{
}

//询问用户是否更新
void MainWindow::isUpdate(QJsonObject &newJSON)
{
    LableSureDialog msg ;
    msg.setWindowTitle("自动更新");
    QString temp = "版本：" + newJSON.value("version").toString() ;
    temp.append("\n更新日志：\n") ;
    temp.append(newJSON.value("info").toString() ) ;
    temp.append("\n以下文件需要更新:\n"+upList.join("\n")) ;
    msg.SetLable("BKE Creator已经有了新版本，是否更新？\n\n"+temp);
    msg.SetCheckbox(QStringList()<<"不再提示自动更新");
    msg.SetBtn(QStringList()<<"【立即更新】"<<"下次再说");

    int ks =  msg.WaitUser() ;
    if( msg.IsCheckboxChoise(0) ){  //关闭自动更新
        BKE_CLOSE_SETTING->setValue("update/close",true);
    }
    if( ks != 0 ) return ;

    startUp();
}

void MainWindow::startUp()
{
#ifdef Q_OS_WIN
    QDesktopServices::openUrl(QUrl::fromLocalFile(BKE_CURRENT_DIR+"/update.exe")) ;
#elif defined(Q_OS_MAC)
    QDesktopServices::openUrl(QUrl::fromLocalFile(qApp->applicationDirPath()+"/update"));
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(BKE_CURRENT_DIR+"/update")) ;
#endif
    close() ;
}

bool MainWindow::hasFileUp(QJsonObject fi)
{
    QByteArray temp ;
    LOLI::AutoRead(temp,BKE_CURRENT_DIR+"/version.txt") ;
    QJsonDocument dc = QJsonDocument::fromJson( temp ) ;
    QJsonObject oldfiles ;
    if( !dc.isNull() ) oldfiles = dc.object().value("files").toObject() ;

    upList.clear();
    QStringList ls = fi.keys() ;
    for( int i = 0 ; i < ls.size() ; i++){
        if( oldfiles.value(ls.at(i)).toString() != fi.value(ls.at(i)).toString()) upList.append( ls.at(i) );
    }
    return upList.size() > 0 ;
}


//事件过滤器
bool MainWindow::eventFilter ( QObject * watched, QEvent * event )
{
    if( watched == this && QEvent::WindowActivate == event->type())
    {  //主窗口被激活，检查文件更新
        if( codeedit->ignoreActive ) codeedit->ignoreActive = false ;
        else codeedit->QfileChange("");
    }
    return false ;
}


//开启或关闭自动更新
void MainWindow::OCupdate()
{
    QString temp ;
    if( BKE_CLOSE_SETTING->value("update/close").toBool() ){
        BKE_CLOSE_SETTING->setValue("update/close",false);
        temp = "自动更新已开启！" ;
    }
    else{
        BKE_CLOSE_SETTING->setValue("update/close",true);
        temp = "自动更新已关闭！" ;
    }

    QMessageBox::information(this,"",temp,QMessageBox::Ok) ;
}

//配置项目
void MainWindow::Config()
{
//    CConfigdia *ctk = new CConfigdia ;
//    ctk->exec();
//	delete ctk;
    SetOptionDia *optiondia = new SetOptionDia() ;
    optiondia->exec() ;

    codeedit->resetLexer();
    delete optiondia ;


}

void MainWindow::CurrentProChange(BkeProject *pro)
{
	if (pro)
	{
		btnReleaseGame->setEnabled(true);
	}
	else
	{
		btnReleaseGame->setEnabled(false);
	}
}

void MainWindow::ReleaseGame()
{
	projectedit->ReleaseCurrentGame();
}