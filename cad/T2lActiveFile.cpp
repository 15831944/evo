//
// Copyright (C) 2015 Petr Talla. [petr.talla@gmail.com]
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//		      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=============================================================================

//cad
#include "T2lActiveFile.h"
#include "T2lGFileObjects.h"
#include "T2lGFileCol.h"

//hg
#include "T2lStoredItem.h"
#include "T2lStoredFactory.h"
#include "T2lUpdateLock.h"

//infrastructure
#include "TcCmdTransl.h"
#include "T2lQueue.h"

//qt
#include <QDir>
#include <QFile>
#include <QCoreApplication>

//std
#include <iostream>
#include <fstream>

using namespace T2l;
using namespace std;

ActiveFile* ActiveFile::instance_ = NULL;

//====================================================================
ActiveFile::ActiveFile()
{
}

//====================================================================
ActiveFile::~ActiveFile()
{
}

//====================================================================
QString ActiveFile::firstFileName_()
{


    QString fileName = QDir::homePath() +  "/Development/KADLUB/cvz/samples/storage/empty.t2l";

#ifdef WINDOWS
    fileName = QDir::homePath() + "g:/home/KADLUB/cvz/samples/storage";
#endif

    cout << endl;

    cout << QCoreApplication::applicationDirPath().toStdString() << endl;

    return fileName;
}

//====================================================================
ActiveFile& ActiveFile::active()
{
    if ( instance_ == NULL) {
        UpdateLock l;

        instance_ = new ActiveFile();
        GFile* file = new GFileObjects(firstFileName_());
        GFileCol::instance().add(file);
        instance_->file_ = file;
        instance_->file()->load();

        TcCmdTransl::xcall("voidcmd", true);
    }

    return *instance_;
}

//====================================================================
bool ActiveFile::changeActiveFile(GFile* file, bool unload)
{
    //cout << "changing file to: " << file->filePath().toStdString() << endl;

    if (instance_) {
        if (unload) instance_->file()->unload();
        delete instance_;
    }

    instance_ = new ActiveFile();
    instance_->file_ = file;
    instance_->file_->load();

    return true;
}

//====================================================================

