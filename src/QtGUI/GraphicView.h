/*                             G R A P H I C V I E W . H
 * BRL-CAD
 *
 * Copyright (c) 2018 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/** @file GraphicView.h
 *
 *  BRL-CAD Qt GUI:
 *      declaration of the graphical visualization
 */

#ifndef GRAPHICVIEW_H
#define GRAPHICVIEW_H

#include <QWidget>
#include <QMatrix4x4>

#include <brlcad/ConstDatabase.h>


class GraphicView : public QWidget {
    Q_OBJECT
public:
    GraphicView(BRLCAD::ConstDatabase& database,
                QWidget*               parent = 0);

public slots:
    void Update(void);
    void UpdateTrafo(const QMatrix4x4& transformation);

protected:
    virtual void paintEvent(QPaintEvent* event);

private:
    BRLCAD::ConstDatabase& m_database;
    QMatrix4x4             m_transformation;
    QImage                 m_image;
    bool                   m_imageUpTodate;
    bool                   m_updatingImage;

    void UpdateImage(void);
};


#endif // GRAPHICVIEW_H
