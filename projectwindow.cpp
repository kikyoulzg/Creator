﻿#include "projectwindow.h"
#include "dia/newprodia.h"
#include <QSize>
#include "dia/lablesuredialog.h"

QList<BKEproject*> projectlist ;


ProjectWindow::ProjectWindow(QWidget *parent)
    :QTreeWidget(parent)
{
    setStyleSheet("QTreeWidget{ border:0px; }");
    setHeaderHidden(true);
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(ItemDoubleClick(QTreeWidgetItem*,int))) ;
    connect(this,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(ShowRmenu(QPoint))) ;

    QStringList ls = QString("设为活动项目 编译脚本 发布游戏 插入路径 预览 新建脚本 新建导入脚本 添加文件 添加目录 在文件夹中显示 在这里搜索 移除 关闭项目").split(" ") ;
    for( int i = 0 ; i < BTN_COUNT ; i++){
        btns[i] = new QAction(ls.at(i),this);
        connect(btns[i],SIGNAL(triggered()),this,SLOT(ActionAdmin())) ;
    }

    workpro = 0 ;
}


void ProjectWindow::NewProject()
{
    NewProDia use(this) ;
    use.WaitUser() ;
    BKEproject *pro = new BKEproject ;
    if( use.type == 0 ){
        pro->NewProject(use.okdir,use.okname) ;
    }
    else if( use.type == 1){

    }
    else
    {
        delete pro;
        return ;
    }

    projectlist << pro ;
    addTopLevelItem(pro->Root);
    BkeChangeCurrentProject(pro);
    BkeCreator::AddRecentProject(pro->FileDir()+"/"+BKE_PROJECT_NAME) ;
}

void ProjectWindow::OpenProject()
{
    QString name = QFileDialog::getOpenFileName(this,"打开项目","","Bke项目(*.bkp)") ;
    OpenProject(name);
}

void ProjectWindow::OpenProject(const QString &file)
{
    if( file.isEmpty() ) return  ;

    QString path = QFileInfo(file).path();
    for(auto it = projectlist.begin();it != projectlist.end();++it)
    {
        if((*it)->FileDir()==path)
        {
            BkeChangeCurrentProject(*it);
            return;
        }
    }
    BKEproject *pro ;

    pro = new BKEproject ;
    if(!pro->OpenProject(file))
    {
        QMessageBox::information(this,"错误","文件不存在，项目打开失败",QMessageBox::Ok) ;
        delete pro;
        return;
    }
    projectlist << pro ;
    addTopLevelItem(pro->Root);
    BkeChangeCurrentProject(pro);



//    //读取书签
//    QString text ;
//    LOLI::AutoRead(text,pro->FileDir()+"/BkeProject.bmk") ;
//    emit TextToMarks(text,pro->FileDir(),0);
    BkeCreator::AddRecentProject(pro->FileDir()+"/"+BKE_PROJECT_NAME) ;
    //默认展开节点
    workpro->Root->setExpanded(true);
    workpro->Script->setExpanded(true);
    workpro->Import->setExpanded(true);
}

//读取项目，并发送
bool ProjectWindow::ReadItemInfo(QTreeWidgetItem *dest,ItemInfo &f)
{
    if( dest == NULL ) return false ;
    BKE_PROJECT_READITEM(dest,f) ;
    return true ;
}

//项目被双击
void ProjectWindow::ItemDoubleClick(QTreeWidgetItem * item, int column)
{
    if( !ReadItemInfo(item,info) ) return ;

    BKEproject *p = FindPro(info.ProName);
    QString name = p->FileDir()+"/"+info.FullName ;

    if( info.FullName == "config.bkpsr" ){
        ConfigProject(p->config);
        return ;
    }

    if( name.endsWith(".bkscr") || name.endsWith(".bkpsr")){
        emit OpenThisFile(name, p->FileDir());
    }


}

//右键菜单
void ProjectWindow::ShowRmenu( const QPoint & pos )
{
    if( !ReadItemInfo(currentItem(),info) ) return ;

    QPoint pt = QCursor::pos() ;
    QMenu mn ;

    if( info.Layer < 1){
        mn.addAction(btns[btn_active]) ;
        mn.addAction(btns[btn_compile]) ;
        mn.addAction(btns[btn_release]) ;
    }
    else if( info.Layer > 1){
        mn.addAction(btns[btn_insertdir]) ;
        mn.addAction(btns[btn_preview]) ;
    }

    mn.addSeparator() ;
    for( int i = btn_newscript ; i <= btn_search ; i++){
        mn.addAction( btns[i] ) ;
        if( i == btn_adddir ) mn.addSeparator() ;
        else if( i == btn_newimport ) mn.addSeparator() ;
    }

    mn.addSeparator() ;
    if( info.Layer < 1){
        mn.addAction(btns[btn_close]) ;
    }
    else if( info.Layer > 1){
        mn.addAction(btns[btn_remove]) ;
    }

    pt.setX(pt.x()+10);
    mn.exec(pt) ;
    return  ;

}

//设置选中项
void ProjectWindow::SetCurrentItem(const QString &file)
{
    BKEproject *abc ;
    QTreeWidgetItem *le ;
    for( int i = 0 ; i < projectlist.size() ; i++){
        abc = projectlist.at(i) ;
        le = abc->FindItem(abc->Import,file,false) ;
        if( le == 0 ) le = abc->FindItem(abc->Script,file,false ) ;
        if( le == 0 ) le = abc->FindItem(abc->Source,file,false ) ;
        if( le != 0 ){
            setCurrentItem(le);
            return ;
        }
    }
}

void ProjectWindow::NewFile(const ItemInfo &f, int type)
{
    QString name = QInputDialog::getText(this,"新建脚本","输入脚本名称，如: \r\n   abc \r\n   abc/sence.bkscr") ;
    if( name.isEmpty()) return ;
    else if( !name.endsWith(".bkscr")) name.append(".bkscr") ;
    name = name.replace(QRegExp("\\"),"/") ;

    BKEproject *p = FindPro(f.ProName);
    QFileInfo sk(p->absName(name)) ;
    if( sk.exists() ){
        int sk = QMessageBox::information(this,"","文件已经存在，是否直接添加文件",QMessageBox::Yes|QMessageBox::No) ;
        if( sk == QMessageBox::No ) return ;
    }
    else LOLI_MAKE_NULL_FILE(sk.filePath()) ;

    if( type == 1){
        p->FindItem(p->Import,name) ;
    }
    else if( type == 2){
        p->FindItem(p->Script,name) ;
    }
    else{
        p->FindItem(p->Source,name) ;
    }

    p->AddFileToHash(p->typeHash(type),name);
    p->WriteBkpFile() ;
}


//寻找项目
BKEproject *ProjectWindow::FindPro(const QString &proname)
{
    BKEproject *abc ;
    for( int i = 0 ; i < projectlist.size() ; i++){
        abc = projectlist.at(i) ;
        if( abc->ProjectName() == proname){
            return abc ;
        }
    }
    return 0 ;
}

QTreeWidgetItem *ProjectWindow::findFileInProject(const QString &name)
{
    //没有项目返回0
    if( projectlist.size() < 1) return 0 ;

    temppro = workpro ;
    QTreeWidgetItem *le = temppro->FindItemAll(name) ;
    if( projectlist.size() == 1) return le ;

    for( int i = 0 ; i < projectlist.size() ; i++){
        temppro = projectlist.at(i) ;
        if( temppro != workpro) le = temppro->FindItemAll(name) ;
        if( le != 0) break ;
    }
    return le ;
}


void ProjectWindow::DeleteFile(const ItemInfo &f)
{
    //是文件的话将询问是否移除文件
    BKEproject *p = FindPro(f.ProName);
    QString sk = p->FileDir()+"/"+f.FullName ;
    LableSureDialog msg;
    msg.SetLable("要移除文件"+sk+"吗？");
    msg.SetCheckbox(QStringList()<<"彻底删除");
    if( p->IconKey(f.IconKey) == "@dir" ) msg.SetCheckboxAble(0,false);
    msg.SetBtn(QStringList()<<"移除"<<"取消");
    if( msg.WaitUser(240,130) == 1) return ;

    if( msg.IsCheckboxChoise(0) ){
        QFile(sk).remove() ;
    }
    //移除文件
    if( !p->RemoveItem(f) ){
        QMessageBox::information(this,"错误","移除项目过程中出了一个错误",QMessageBox::Ok) ;
        return ;
    }

    //写出结果
    p->WriteBkpFile() ;
}

void ProjectWindow::Addfiles(const ItemInfo &f)
{
    BKEproject *p = FindPro(f.ProName);
    QStringList ls ;
    if( f.RootName == "初始化" || f.RootName == "脚本"){
        ls = QFileDialog::getOpenFileNames(this,"添加文件",p->FileDir(),"bkscr脚本(*.bkscr)") ;
    }
    else ls = QFileDialog::getOpenFileNames(this,"添加文件",p->FileDir(),"所有文件(*.*)") ;

    if( ls.isEmpty() ) return ;
    QStringList errors;
    auto it = ls.begin();
    while(it != ls.end())
    {
        QString rfile = BkeFullnameToName( *it, p->FileDir() );
        if(rfile.isEmpty())
        {
            errors.append(*it);
            it = ls.erase(it);
        }
        else
        {
            *it = rfile;
            it++;
        }
    }
    if(!errors.isEmpty())
    {
        QMessageBox::warning(this,"警告","不允许添加工程目录之外的文件：\n" + errors.join('\n'));
    }
    p->Addfiles(ls,f);
}

//
void ProjectWindow::AddDir(const ItemInfo &f)
{
    BKEproject *p = FindPro(f.ProName);
    QString d = QFileDialog::getExistingDirectory(this,"添加目录",p->FileDir()) ;
    if(d.isEmpty())
        return;
    QString tmp = BkeFullnameToName(d,p->FileDir());
    if( tmp.isEmpty() ){
        QMessageBox::information(this,"","工作目录之外的文件不能添加",QMessageBox::Ok) ;
        return ;
    }
    p->AddDir(tmp,f);
}

void ProjectWindow::ReName()
{
    QString name = QInputDialog::getText(this,"重命名","将要改变到的名称，后缀名可以省略。不能含有\/:*?\"<>|") ;
    if( name.isEmpty() ) return ;
    else if( !name.endsWith(".bkscr")) name.append(".bkscr") ;

    QString oldname = workpro->FileDir()+"/"+info.FullName ;
    QString newname = oldname.left(oldname.lastIndexOf("/")) + "/" + name ;

    QFile temp(newname) ;
    if( temp.exists() ){
        QMessageBox::information(this,"重命名","文件已存在，不能再使用这个名称。",QMessageBox::Ok) ;
        return ;
    }

    bool result ;
    emit FileNameChange(oldname,newname,result);
    if( !result){
        QMessageBox::warning(this,"重命名","重命名失败") ;
        return ;
    }

    QTreeWidgetItem *le = currentItem() ;
    le->setText(0,name);
    workpro->WriteBkpFile() ;
}


void ProjectWindow::OpenFile()
{
    QString filename = QFileDialog::getOpenFileName(this,"选择要打开的文件","","bksfile(*.bkscr);;Allfile(*.*)") ;
    if( filename.isNull() ) return ;

    QTreeWidgetItem *llm = findFileInProject(filename) ;
    if( llm == 0) emit OpenThisFile(filename,"");
    else OpenThisFile(filename,temppro->FileDir());
}

void ProjectWindow::BkeChangeCurrentProject(BKEproject *p)
{
    if( workpro != 0){
        workpro->SetTopLeveBold(false);
    }

    if( p != 0){
        workpro = p ;
        workpro->SetTopLeveBold(true);
        emit CurrentProChange(workpro);
    }
    else{
        workpro = 0 ;
        emit CurrentProChange(workpro);
    }
}

BKEproject *ProjectWindow::FindFileProject(const QString &file)
{
    if( findFileInProject(file) != 0 ) return workpro ;
    else return 0 ;
}

BKEproject *ProjectWindow::FindProjectFromDir(const QString &dir)
{
    QString a = LOLI_OS_QSTRING( dir ) ;
    for( int i = 0 ; i< projectlist.size() ; i++){
        if( a == LOLI_OS_QSTRING(projectlist.at(i)->FileDir()) ) return projectlist.at(i) ;
    }
    return 0 ;
}

void ProjectWindow::ConfigProject(BkeProjectConfig *config)
{
    BkeConfigUiModel kag ;
    kag.StartConfig(config);
}

void ProjectWindow::ActionAdmin()
{
    QAction *p = dynamic_cast<QAction*>(sender()) ;

    if( p == NULL ) return ;
    else if( p == btns[btn_active] ) Active(info);
    else if( p == btns[btn_compile] ) emit Compile();
    else if( p == btns[btn_release] ) ;
    else if( p == btns[btn_insertdir] ) emit DirWillBeInsert(info.FullName);
    else if( p == btns[btn_preview] ) PreviewFile(info);
    else if( p == btns[btn_newscript] ) NewFile(info, 2);
    else if( p == btns[btn_newimport] ) NewFile(info, 1);
    else if( p == btns[btn_addfile] ) Addfiles(info);
    else if( p == btns[btn_adddir] ) AddDir(info);
    else if( p == btns[btn_showindir] ) ShowInDir(info);
    else if( p == btns[btn_search]) ;
    else if( p == btns[btn_remove]) DeleteFile(info);
    else if( p == btns[btn_close]) CloseProject(info);
}

void ProjectWindow::PreviewFile(const ItemInfo &f)
{
    BKEproject *p = FindPro(f.ProName);
    QString n = p->FileDir()+"/"+info.FullName ;
    if( n.endsWith(".bkscr") || n.endsWith(".bkpsr") ) emit OpenThisFile(n,p->FileDir());
    else{
        QDesktopServices::openUrl(QUrl::fromLocalFile(n)) ;
    }
}

void ProjectWindow::CloseProject(const ItemInfo &f)
{
    BKEproject *p = FindPro(f.ProName);
    if(p)
    {
        projectlist.removeOne(p);
        p->deleteLater();
        takeTopLevelItem(indexOfTopLevelItem(p->Root));
        if(p == workpro)
        {
            p = NULL;
            if(!projectlist.empty())
            {
                p = projectlist.first();
            }
            BkeChangeCurrentProject(p);
        }
    }
}

void ProjectWindow::Active(const ItemInfo &f)
{
    BKEproject *p = FindPro(f.ProName);
    if(p)
    {
        BkeChangeCurrentProject(p);
    }
}

#if defined(Q_OS_WIN)
#include <windows.h>
#include <shellapi.h>
#pragma comment(lib,"shell32.lib")
#endif

void ProjectWindow::ShowInDir(const ItemInfo &f)
{
    BKEproject *p = FindPro(f.ProName);
    QString n = p->FileDir()+"\\"+info.FullName ;
#if defined(Q_OS_WIN)
    n.replace('/','\\');
    QByteArray a = ("/select,"+n).toLocal8Bit();
    ShellExecuteA(NULL,"open","explorer.exe",a.data(),NULL,true);
#endif
}
