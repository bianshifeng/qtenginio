/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://qt.digia.com/contact-us
**
** This file is part of the Enginio Qt Client Library.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
****************************************************************************/

#ifndef ENGINIOOBJECTOPERATION_H
#define ENGINIOOBJECTOPERATION_H

#include "enginiooperation.h"

class EnginioAbstractObject;
class EnginioObjectModel;
class EnginioObjectOperationPrivate;

class ENGINIOCLIENT_EXPORT EnginioObjectOperation : public EnginioOperation
{
    Q_OBJECT
public:
    explicit EnginioObjectOperation(EnginioClient *client,
                                    EnginioObjectModel *model = 0,
                                    QObject *parent = 0);
    virtual ~EnginioObjectOperation();

    EnginioObjectModel * model() const;
    void setModel(EnginioObjectModel *model);
    QModelIndex modelIndex() const;
    void setModelIndex(QModelIndex index);

    void create(EnginioAbstractObject *object);
    void create(const EnginioAbstractObject &object);

    void read(EnginioAbstractObject *object);
    void read(const EnginioAbstractObject &object);
    EnginioAbstractObject * read(const QString &id, const QString &objectType);

    void update(EnginioAbstractObject *object);
    void update(const EnginioAbstractObject &object);

    void remove(EnginioAbstractObject *object);
    void remove(const EnginioAbstractObject &object);
    void remove(const QString &id, const QString &objectType);

    QString objectId() const;

signals:
    void objectUpdated() const;

protected:
    EnginioObjectOperation(EnginioClient *client,
                           EnginioObjectModel *model,
                           EnginioObjectOperationPrivate &dd,
                           QObject *parent = 0);

private:
    Q_DECLARE_PRIVATE(EnginioObjectOperation)
    Q_DISABLE_COPY(EnginioObjectOperation)
};

#endif // ENGINIOOBJECTOPERATION_H
