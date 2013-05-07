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

#ifndef ENGINIOQMLCLIENT_H
#define ENGINIOQMLCLIENT_H

#include "enginioclient.h"
#include <QQmlParserStatus>

class EnginioQmlAclOperation;
class EnginioQmlIdentityAuthOperation;
class EnginioQmlObjectModel;
class EnginioQmlObjectOperation;
class EnginioQmlQueryOperation;

class EnginioQmlClient : public EnginioClient
{
    Q_OBJECT
    Q_DISABLE_COPY(EnginioQmlClient)

public:
    EnginioQmlClient(const QString &backendId = QString(),
                     const QString &backendSecret = QString(),
                     QObject *parent = 0);

    Q_INVOKABLE EnginioQmlObjectOperation * createObjectOperation(
            EnginioQmlObjectModel *model = 0);
    Q_INVOKABLE EnginioQmlQueryOperation * createQueryOperation(
            EnginioQmlObjectModel *model = 0);
    Q_INVOKABLE EnginioQmlIdentityAuthOperation * createIdentityAuthOperation();
    Q_INVOKABLE EnginioQmlAclOperation * createAclOperation();
};

#endif // ENGINIOQMLCLIENT_H

