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
#include "T2lCmd_object_trimtoother.h"
#include <T2lScene.h>
#include <T2lUpdateLock.h>
#include <T2lRef.h>
#include <T2lDisplay.h>
#include <T2lRefColSelection.h>
#include <T2lGObject.h>
#include <T2lGObjectPool.h>
#include "T2lObjectDisplable.h"
#include "T2lPoint2.h"
#include "T2lCadLine.h"
#include "T2lEntityLine.h"
#include "T2lCadSettings.h"
#include "T2lRay2.h"

using namespace T2l;

//===================================================================
Cmd_object_trimtoother::Cmd_object_trimtoother(void) :
    Cmd("trim to"),
    cadLine1_trimWhat_(NULL),
    cadLine2_trimTo_(NULL)
{
}

//===================================================================
Cmd_object_trimtoother::~Cmd_object_trimtoother(void)
{
}

//===================================================================
CadLine* Cmd_object_trimtoother::selectLine_( const Point2F& pt, Display& view )
{
    EntityPack* pack = view.entityPack();

    int count = pack->scene()->count();

    for ( long i = 0; i < count; i++ )
    {	Ref* ref = pack->scene()->get(i);
        CadLine* cadLine = dynamic_cast<CadLine*>(ref->object());
        if (cadLine == NULL) continue;
        if (cadLine == cadLine1_trimWhat_) continue;

        if ( ref->identifiedByPoint(view.getRefCanvas(), pt) )
        {	ref->object()->isSelectedSet(true);
            return cadLine;
        }
    }
}

//===================================================================
void Cmd_object_trimtoother::enterReset( Display& /*view*/ )
{
    cadLine1_trimWhat_ = NULL;
    cadLine2_trimTo_   = NULL;
}

//===================================================================
void Cmd_object_trimtoother::enterPoint( const Point2F& pt, Display& view )
{	//<STEP> Searching scene.

    EntityPack* pack = view.entityPack();
    if ( pack == NULL ) { assert(0); return; }
    if ( pack->scene() == NULL ) return;

    UpdateLock l;

    RefColSelection& selected = GObjectPool::instance().selected();

    pack->cleanDynamic();

    if ( cadLine1_trimWhat_ == NULL ) {
        cadLine1_trimWhat_ = selectLine_(pt, view);
    }
    else if ( cadLine2_trimTo_ == NULL ) {
        cadLine2_trimTo_ = selectLine_(pt, view);

        if ( cadLine2_trimTo_ ) {
            Point2F pt0;
            Point2F pt1;

            calculateIntersection_(pt0, pt1);

            EntityLine* line = new EntityLine( Color(255, 0, 255), 2 );
            line->points().points().add( pt0 );
            line->points().points().add( pt1 );
            pack->addDynamic(line);

            cadLine1_trimWhat_->points().getRef(0) = pt0;
            cadLine1_trimWhat_->points().getRef(1) = pt1;
        }
    }
    else
    {
        selected.unselectAll();
        cadLine1_trimWhat_ = NULL;
        cadLine2_trimTo_ = NULL;
    }

    pack->dynamicRefresh();
}

//===================================================================
void Cmd_object_trimtoother::calculateIntersection_(Point2F& pt0, Point2F& pt1)
{
    Point2F PT0 = cadLine1_trimWhat_->points().get(0);
    Point2F PT1 = cadLine1_trimWhat_->points().get(1);

    Point2F PTto0 = cadLine2_trimTo_->points().get(0);
    Point2F PTto1 = cadLine2_trimTo_->points().get(1);

    Ray2 line_trimWhat(PT0, PT1);
    Ray2 line_trimTo(PTto0, PTto1);

    double intersect;
    bool exists = line_trimWhat.intersectParam(line_trimWhat, line_trimTo, intersect);

    if (exists == false) {
        pt0 = PT0;
        pt1 = PT1;
        return;
    }

    Point2F pt = line_trimWhat.getPoint(intersect);

    if ( Vector2F(pt, PT0).getLengthSq() < Vector2F(pt, PT1).getLengthSq() ) {
        pt0 = pt;
        pt1 = PT1;
    }
    else {
        pt0 = PT0;
        pt1 = pt;
    }
}

//===================================================================
void Cmd_object_trimtoother::enterMove( const Point2F& pt, Display& view )
{
    if (cadLine1_trimWhat_ == NULL) return;
    if (cadLine2_trimTo_   == NULL) return;

    EntityPack* pack = view.entityPack();
    if ( pack == NULL ) { assert(0); return; }

}

//===================================================================