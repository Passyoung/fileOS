#include<stack>
#include<iostream>
#include<iomanip>
#include<queue>
#include <fstream>
#include "DiskManager.h"
#include "Folder.h"
#include "FileType.h"
#include "FAT.h"
#include <unistd.h>
#include <sys/stat.h>

const string ACCESS[] = {"ֻ��", "���޸�", "��ִ��"};
const string rootPath = "/Users/";
queue<FCB *> persistQueue;//�־û�����
FAT fat;
string blocks[N];
ofstream *out = NULL;
ifstream *in = NULL;


using namespace std;


void DiskManager::DiskWrite(File *file) {
    //�ļ������
    printf("%s\n", file->path.c_str());
    out = new ofstream(file->path.c_str());
    if (out->is_open()) {
        out->close();
    }
}

bool DiskManager::DiskMkdir(string dirName) {
    printf("%s\n", dirName.c_str());
    #ifdef __linux__
        return mkdir(dirName.c_str(), S_IRWXU) == 0;
    #else
        return mkdir(dirName.c_str()) == 0;
    #endif

}

bool DiskManager::DiskRmdir(string dirName) {
    return rmdir(dirName.c_str()) == 0;
}

bool DiskManager::DiskCkdir(string dirName) {

    if (access(dirName.c_str(), 0) == -1) {
        #ifdef __linux__
                return mkdir(dirName.c_str(), S_IRWXU) == 0;
        #else
                return mkdir(dirName.c_str()) == 0;
        #endif
    }
    return false;
}

void DiskManager::DiskRmdir(Folder *f) {
    //DFSɾ��
    for (int i = 0; i < f->child.size(); i++) {
        //�ж��ļ�����
        if (f->child[i]->type == DOCUMENT) {
            printf("%s\n", f->child[i]->path.c_str());
            remove(f->child[i]->path.c_str());
            delete f->child[i];
        } else {
            this->DiskRmdir((Folder *) f->child[i]);
        }
    }
    printf("%s\n", f->path.c_str());
    this->DiskRmdir(f->path.c_str());
}

DiskManager::DiskManager() {
    fat.init(blocks);
    root = new Folder(rootPath, FileType::FOLDER);
    root->path = rootPath;
    this->DiskMkdir(rootPath);
    //���ô��̸�ΪĿ¼
    //���ø��ڵ�ĸ��ڵ�Ϊ����
    root->father = root;
    cout << " ��ӭ����-----------��������help��ð���------------" << endl << "\n[root@localhost " + rootPath + "]# ";
    string opear, cmd;
    while (cin >> cmd) {
        if (cmd == "format") {
            this->format(blocks);
        } else if (cmd == "mkdir") {
            this->Mkdir();
        } else if (cmd == "rmdir") {
            this->Rmdir();
        } else if (cmd == "ls") {
            this->ls();
        } else if (cmd == "cd") {
            this->cd();
        } else if (cmd == "create") {
            this->create();
        } else if (cmd == "open") {
            this->open();
        } else if (cmd == "close") {
            this->close();
        } else if (cmd == "rm") {
            this->rm();
        } else if (cmd == "exit") {
            printf("%s\n", "�ټ���");
            break;
        } else if (cmd == "help") {
            cout << "\n��format:���ļ��洢�����и�ʽ��.\n" <<
                 "��mkdir:���ڴ�����Ŀ¼\n" <<
                 "��rmdir : ����ɾ����Ŀ¼\n" <<
                 "��ls : ������ʾĿ¼\n" <<
                 "��cd : ���ڸ��ĵ�ǰĿ¼\n" <<
                 "��create : ���ڴ����ļ�\n" <<
                 "��open : ���ڴ��ļ�\n" <<
                 "��close : ���ڹر��ļ�\n" <<
                 "��write : ����д�ļ�\n" <<
                 "��read : ���ڶ��ļ�\n" <<
                 "��rm : ����ɾ���ļ�\n" <<
                 "��exit : �˳�ϵͳ\n"
                 << endl;
        } else {
            cout << "����ָ��������������룡��" << endl;
        }
        cout << "\n[root@localhost " + this->root->path + " ]# ";
    }
}

DiskManager::~DiskManager() {

}

void DiskManager::format(string *blocks) {
    fat.init(blocks);

    //���˵���Ŀ¼
    while (root->father != root) {
        this->root = (Folder *) (this->root->father);
    }
    this->DiskRmdir(this->root);
    root->child.clear();
    printf("%s\n", "���̸�ʽ���ɹ���");
}

void DiskManager::Mkdir() {
    string name;
    cin >> name;
    Folder *childFile = new Folder(name, FileType::FOLDER);
    //���ø��ڵ�
    childFile->father = (this->root);
    childFile->path = this->root->path + name + "/";
    //�ж��Ƿ��ļ��ظ�
    if (this->root->count(childFile)) {
        //�ļ��ظ�����
        cout << "�����ļ���ʧ�ܣ��ļ����������ظ�" << endl;
    } else {
        cout << "�����ļ��гɹ�" << endl;
        this->DiskMkdir(childFile->path);
        this->root->addChild(childFile);
    }
}

void DiskManager::Rmdir() {
    string name;
    cin >> name;
    Folder *childFile = new Folder(name, FOLDER);
    childFile = (Folder *) this->root->find(childFile);
    if (this->root->erase(childFile)) {
        //�ļ��ظ�����
        this->DiskRmdir(childFile);
        cout << "ɾ���ļ��гɹ�" << endl;
    } else {
        cout << "�޴��ļ��� ��ɾ���ļ���ʧ��" << endl;
    }
}

void DiskManager::ls() {

    cout << setw(10) << "����Ȩ��"<< setw(20) << "�ļ���С"<< setw(25) << "�޸�����" << setw(20) << "�ļ���"<< endl;
    int size = this->root->size();
    for (int i = 0; i < size; i++) {
        cout << setw(10) << ACCESS[this->root->child[i]->access]
             << setw(20)
             << (this->root->child[i]->type != FOLDER ? ((File *) this->root->child[i])->toString(blocks).size() : 4096)
             << setw(25) << this->root->child[i]->modifyDate
             << setw(20) << this->root->child[i]->name
             << endl;
    }
}

void DiskManager::cd() {
    string name;
    cin >> name;
    if (name == "..") {
        this->root = (Folder *) (this->root->father);
    } else {
        if (this->root->count(new Folder(name, FOLDER))) {

            if (this->root->find(new Folder(name, FOLDER))->type != FOLDER) {
                cout << "�޴��ļ���" << endl;
            } else {
                root = (Folder *) this->root->find(new Folder(name, FOLDER));
            }
        } else {
            cout << "�޴��ļ��� " << endl;
        }
    }

}

void DiskManager::create() {
    string name;
    cin >> name;

    File *childFile = new File(name, DOCUMENT, fat);
    //���ø��ڵ�
    childFile->father = (this->root);
    childFile->path = this->root->path + name;
    //�ж��Ƿ��ļ��ظ�
    if (this->root->count(childFile)) {
        //�ļ��ظ�����
        cout << "�����ļ�ʧ�ܣ��ļ��������ظ�����" << endl;
    } else {
        cout << "�����ļ��ɹ���" << endl;
        this->root->addChild(childFile);
        this->DiskWrite(childFile);
    }
}

void DiskManager::open() {
    string name, cmd;
    cin >> name;

    File *file = (File *) this->root->find(new File(name, DOCUMENT, fat));
    if (file != NULL) {
        printf("%s\n", "�ļ���д���򿪳ɹ�!");
        cout << "\n[root@localhost " + this->root->path + " ]# ";
        while (cin >> cmd) {
            cout << "\n[root@localhost " + this->root->path + " ]# ";
            if (cmd == "write") {
                this->write(file->path.c_str(), file);
            } else if (cmd == "read") {
                this->read(file->path.c_str());
            } else if (cmd == "close") {
                this->close();
                break;
            }
        }
    } else {
        printf("%s\n", "�޷����ļ���д�����޴��ļ���");
    }
}

void DiskManager::close() {
    if (out == NULL || in == NULL) {
        printf("%s\n", "���ļ���д����Ҫ�ر�!");
    } else {
        out->close();
        in->close();
        printf("%s\n", "�ļ���д���رճɹ�!");
    }
}

void DiskManager::write(const char *s, File *file) {
    string content;
    cin >> content;
    if (in != NULL)in->close();
    file->addContent(content.c_str(), blocks, fat);//������ݵ��ļ���
    content = file->toString(blocks);
    out = new ofstream(s);
    if (out->is_open()) {
        *out << content;
    }
    out->close();
}

void DiskManager::read(const char *s) {
    char *content = new char[N];
    if (out != NULL)out->close();
    in = new ifstream(s);
    if (in->is_open()) {
        *in >> content;
    }
    in->close();
    cout << content;
}

void DiskManager::rm() {
    string name;
    cin >> name;
    File *childFile = new File(name, DOCUMENT, fat);
    if (this->root->count(childFile)) {
        //�ļ��ظ�����
        childFile = (File *) this->root->find(childFile);
        remove(childFile->path.c_str());
        childFile->release(fat, blocks);
        this->root->erase(childFile);
        cout << "ɾ���ļ��ɹ���" << endl;
    } else {
        cout << "�޴��ļ� ��ɾ���ļ�ʧ��" << endl;
    }
}
