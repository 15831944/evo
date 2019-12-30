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
#pragma once

#include <T2lCmd.h>
#include <QStringList>

namespace T2l {

//===================================================================
class Cmd_draw_image : public T2l::Cmd {
//===================================================================
public:
	Cmd_draw_image(void);
    virtual ~Cmd_draw_image(void);
//===================================================================
    virtual void enterPoint( const T2l::Point2F& pt, T2l::Display& view );
    virtual void enterMove ( const T2l::Point2F& pt, T2l::Display& view );
    virtual QString dialogTml() const;

    bool    ptEntered_;
    Point2F pt_;

    QStringList getFilesUp_(const QString& dirPath, const QStringList& extensions) const;
};

} //namespace T2l
