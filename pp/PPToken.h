/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
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
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#ifndef CPLUSPLUS_INTERNAL_PPTOKEN_H
#define CPLUSPLUS_INTERNAL_PPTOKEN_H

#include <CPlusPlus.h>
#include <Token.h>
#include <assert.h>

#include <rct/String.h>

namespace CPlusPlus {

class CPLUSPLUS_EXPORT StringRef
{
public:
    StringRef()
        : m_start("")
        , m_length(0)
    {}

    StringRef(const String *ref)
        : m_start(ref->constData())
        , m_length(ref->length())
    {}

    StringRef(const char *start, int length)
        : m_start(start)
        , m_length(length)
    {}

    StringRef(const String *ref, int offset, int length)
        : m_start(ref->constData() + offset)
        , m_length(length)
    {
        assert(ref);
        assert(offset >= 0);
        assert(length >= 0);
        assert(offset + length <= ref->size());
    }

    inline const char *start() const
    { return m_start; }

    inline int length() const
    { return m_length; }

    inline int size() const
    { return length(); }

    inline char at(int pos) const
    { return pos >= 0 && pos < m_length ? m_start[pos] : '\0'; }

    inline char operator[](int pos) const
    { return at(pos); }

    String toString() const
    { return String(m_start, m_length); }

    bool operator==(const String &other) const
    { return m_length == other.length() && !strncmp(m_start, other.constData(), m_length); }
    bool operator!=(const String &other) const
    { return !this->operator==(other); }

    bool operator==(const char *other) const
    { return m_length == (int) strlen(other) && !strncmp(m_start, other, m_length); }
    bool operator!=(const char *other) const
    { return !this->operator==(other); }

    bool startsWith(const char *ch) const;

    int count(char c) const;

private:
    const char *m_start;
    const int m_length;
};

inline bool operator==(const String &other, const StringRef &ref)
{ return ref == other; }

inline bool operator!=(const String &other, const StringRef &ref)
{ return ref != other; }

namespace Internal {

class CPLUSPLUS_EXPORT PPToken: public Token
{
public:
    PPToken() {}

    PPToken(const String &src)
        : m_src(src)
    {}

    void setSource(const String &src)
    { m_src = src; }

    const String &source() const
    { return m_src; }

    bool hasSource() const
    { return !m_src.isEmpty(); }

    void squeezeSource();

    const char *bufferStart() const
    { return m_src.constData(); }

    const char *tokenStart() const
    { return bufferStart() + offset; }

    StringRef asStringRef() const
    { return StringRef(&m_src, offset, length()); }

private:
    String m_src;
};

} // namespace Internal
} // namespace CPlusPlus

#endif // CPLUSPLUS_INTERNAL_PPTOKEN_H
