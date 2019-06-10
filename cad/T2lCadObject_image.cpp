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
//self
#include "T2lCadObject_image.h"

#include "T2lScene.h"
#include "T2lDisplay.h"

//cad
#include "T2lGFile.h"
#include "T2lActiveFile.h"
#include "T2lStoredItem.h"
#include "T2lStoredAttrSTR.h"
#include "T2lStoredAttrNUM.h"
#include "T2lGFileImgDescr.h"

//hg
#include <T2lStyle.h>
#include "T2lEntityList.h"

#include "T2lEntityImage.h"
#include "T2lEntityText.h"
#include "T2lEntityArea.h"
#include "T2lSfeatArea.h"

//infrastructure
#include "T2lBox2.h"

//qt
#include <QPixmap>
#include <QFileInfo>

//std
#include <iostream>
#include <sstream>

using namespace T2l;
using namespace std;

#define T2L_IMAGE_SIZE 1000

//===================================================================
CadObject_image::CadObject_image(const char* fileName, const Point2FCol& position, GFile* parent) :
    ObjectDisplable(Point2Col<double>(position), parent),
    imageName_(fileName),
    pixmap_(NULL)
{
    if (parent == NULL) return;

    QString fileQualified = parent_->qualifiedFilePath(imageName_.toStdString().c_str());
    QFileInfo fi(fileQualified);

    if ( fi.exists() ) {

        /*int w = T2L_IMAGE_SIZE;
        int h = T2L_IMAGE_SIZE;

        int pw = pixmap()->width();
        int ph = pixmap()->height();

        QPixmap pixmap(fileQualified);
        if ( pw > ph) {
            w = T2L_IMAGE_SIZE;
            h = (int)(T2L_IMAGE_SIZE*ph/pw);
        }
        else {
            w = (int)(T2L_IMAGE_SIZE*pw/ph);
            h = T2L_IMAGE_SIZE;
        }
        pixmap = pixmap.scaled(w, h);*/
        pixmap_ = new QPixmap(fileQualified);
    }

    if (parent != NULL) parent->add(this);
}

//===================================================================
CadObject_image::~CadObject_image(void)
{
    delete pixmap_;
}

//===================================================================
bool CadObject_image::loadFromStored( StoredItem* item, GFile* parent )
{
    StoredAttr* type = item->get("type");
    if (type->getAsSTR() == NULL) return false;
    if (type->value() != "entity")  return false;

    StoredAttr* entity = item->get("entity");
    if (entity->getAsSTR() == NULL) return false;
    if (entity->value() != "symbol_image")  return false;

    Point2FCol position;

    StoredAttr* pa = item->get("point", 0);
    if (pa != NULL) {
        StoredAttrNUM* p = pa->getAsNUM();
        if (p != NULL) {
            Point2<double> p2( p->get(0), p->get(1));
            position.add(p2);
        }
    }

    StoredAttr* image_name = item->get("image_name");
    if (image_name == NULL) return false;
    if (image_name->getAsSTR() == NULL) return false;

    string imageName = image_name->value().toStdString();
    CadObject_image* objectImage = new CadObject_image( imageName.c_str(), position, parent );
}

//===================================================================
void CadObject_image::saveToStored(StoredItem& item, GFile* file)
{
    GFileImgDescr* fileImgDescr = dynamic_cast<GFileImgDescr*>(parent());
    if (fileImgDescr) return; //image was loaded based on file name coincidence

    cout << "object image" << endl;

    item.add( new StoredAttrSTR("type",       "entity") );
    item.add( new StoredAttrSTR("entity",     "symbol_image") );
    item.add( new StoredAttrSTR("image_name", imageName_) );

    if ( points_.count() > 0 ) { //TODO
        StoredAttrNUM* attrBeg = new StoredAttrNUM("point");
        Point2F position = points_.get(0);
        attrBeg->add(position.x());
        attrBeg->add(position.y());
        item.add(attrBeg);
    }
}

//=========================================================================
Box2F CadObject_image::boundDisplable()
{
    return box();
}

//===================================================================
Box2F CadObject_image::box(Vector2F offsetArg)
{
    double size = 6;

    Vector2F offset = parent()->getOffset();
    offset = Vector2F( offset.x()+offsetArg.x(),
                       offset.y()+offsetArg.y());


    if (points_.count() > 0) {
        Box2F result;
        Point2F center(points_.get(0));
        center.add(offset);
        result.inflateTo(center);
        result.inflateBy(size);

        return result;
    }
    else {
        return Box2F( IntervalF(offset.x(),                   offset.x()+pixmap_->width()),
                      IntervalF(offset.y()-pixmap_->height(), offset.y()) );
    }
}

//===================================================================
void CadObject_image::display(EntityList& list, RefCol* scene)
{
    if (parent_ == NULL) {
        return;
    }

    Box2F imageBox = box(Vector2F(0, 0));

    EntityImage* eimage = new EntityImage(imageBox);
    eimage->layerSet(-1);
    eimage->pixmapSet(pixmap_, false);
    list.add(eimage);

    if (isSelected()) {
        Style* style = new Style("");
        style->sfeats().add( new SfeatArea(Color(255, 0, 255), 50));

        Box2F boxS = imageBox;
        boxS.inflateBy(1);

        EntityArea* entityArea = new EntityArea( *style, true, NULL );
        for (int i = 0; i < 4; i++) {
            entityArea->points().points().add( boxS.getPoint(i) );
        }

        list.add(entityArea);
    }

    /*EntityText( const QString& text, const Point2F& position,
                EPositionH alignH = POSITION_H_LEFT, EPositionV alignV = POSITION_V_TOP,
                AngleXcc angle = AngleXcc(0),
                const Point2F& offset = Point2F(0, 0), StyleChange* styleChange = NULL );*/

    EntityText* textEnt = new EntityText( text(), imageBox.getPoint(0) );
    list.add(textEnt);
}

//===================================================================
GObject::EIdentified CadObject_image::identifiedByPoint(const Canvas& canvas, const Point2F& pt)
{
    if (parent_ == NULL) return IDENTIFIED_NO;
    if ( box().isInside(pt) ) return IDENTIFIED_YES;
    return IDENTIFIED_NO;
}

//===================================================================
std::string CadObject_image::print()
{
    stringstream ss;

    ss << "Object IMAGE " << "position: ";
    for (int i = 0; i < points_.count(); i++) {
        ss << "xy: ";
        Point2F pti = points_.get(i);
        ss << pti.x() << " " << pti.y();
    }

    ss << "wh: " << pixmap()->width() << "x" << pixmap()->height() << " pix";
    ss << "name" << imageName().toStdString().c_str();

    ss << endl;

    return ss.str();
}

//=========================================================================