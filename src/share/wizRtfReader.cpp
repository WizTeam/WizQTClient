#include "wizRtfReader.h"
#include <QFile>
#include <QTextCodec>
#include <QDebug>
//#include <QByteArrayData>

#include "rtf2html/rtf2html.h"

CWizRtfReader::CWizRtfReader()
{
}

QByteArray file_load (const QString &fileName)
{
  QFile file (fileName);
  QByteArray b;

  if (! file.open (QFile::ReadOnly))
      return b;

  b = file.readAll();
  return b;
}

//www.codeguru.com/forum/archive/index.php/t-201658.html
//rewritten by Peter Semiletov
QString rtf_strip (const QString &rtf)
{
  int length = rtf.size();

  if (length < 4)
     return QString();

  int start = 0;

  start = rtf.indexOf ("\\pard");
  if ( start < 1)
     return QString();

  QString strCopy;
  strCopy.reserve (length);

  int k = 0;

  bool slash = false; //is backslash followed by the space
  bool figure_opened = false; //is opening figure brace followed by the space
  bool figure_closed = false; //is closing brace followed by the space
  bool first_space = false; //else spaces are in plain text and must be included to the result

  QChar ch;
  for (int j = start; j < length; j++)
      {

       ch = rtf.at (j);

       if (ch == '\\')//we are looking at the backslash
          {
           first_space = true;
           slash = true;
          }
       else
       if (ch == '{')
          {
           first_space = true;
           figure_opened = true;
          }
      else
      if (ch == '}')
         {
          first_space = true;
          figure_closed = true;
         }
      else
      if (ch == ' ')// &&
          //(rtf.indexOf ("\\datafield", j - 10) + 10) != j)
         {
          slash = false;
          figure_opened = false;
          figure_closed = false;
         }

      if ( ch == '\\')
         {
          QChar chr = rtf.at (j + 1);

          if (chr == '{') //if the text contains symbol '{'
           {
            slash = false;
            figure_opened = false;
            figure_closed = false;
            first_space = false;
            strCopy += '{';
            j++;
            k++;
            continue;
           }

      if (chr == '}') //if the text contains symbol '}'
         {
          slash = false;
          figure_opened = false;
          figure_closed = false;
          first_space = false;
          strCopy += '}';
          j++;
          k++;
          continue;
         }

      if (chr == '\\')//if the text contains symbol '\'
         {
          slash = false;
          figure_opened = false;
          figure_closed = false;
          first_space = false;
          strCopy += '\\';
          j++;
          continue;
         }
    }

    if (rtf.at (j) == '\\' &&
        rtf.at (j + 1) == 'p' &&
        rtf.at (j + 2) == 'a' &&
        rtf.at (j + 3) == 'r' &&
        rtf.at (j + 4) != 'd')
       {
        slash = false;
        figure_opened = false;
        figure_closed = false;
        first_space = false;
        strCopy += '\n';
        j += 4;
        continue;
       }

    if (slash == false &&
        figure_opened == false &&
        figure_closed == false &&
        ch != '\n')
        {
         if (! first_space)
            strCopy += ch;
         else
            first_space = false;
        }
   }

  return strCopy;
}

bool CWizRtfReader::load(const QString& strFile, QString& strText)
{
    QByteArray ba = file_load (strFile);

    QString text;
    text.reserve (ba.size());

    int i = 0;
    int l = ba.size();

    QString ansicgp;
    int n = ba.indexOf ("ansicpg");
    if (n != -1)
    {
        int m = ba.indexOf ('\\', n);
        n += 7;
        ansicgp = ba.mid (n, m - n);
    }

    if (ansicgp.isEmpty()) //assuming unicode
    {
        while (i < l)
            if ((ba.at(i) == '\\') && (ba.at(i + 1) == 'u'))
            {
                QByteArray ta = ba.mid (i, 7);
                ta = ta.mid (2, 4);
                QChar c (ta.toInt());
                text.append (c);
                i += 7 + 3;
            }
            else
            {
                text.append (ba.at(i));
                i++;
            }
    }
    else
    {
        ansicgp.prepend ("CP");

        QTextCodec *codec = QTextCodec::codecForName (ansicgp.toUtf8().data());
        qDebug() << "not unicode!";

        while (i < l)
            if ((ba.at(i) == '\\') && (ba.at(i + 1) == '\''))
            {
                QByteArray ta = ba.mid (i, 4);
                ta = ta.mid (2, 2);
                QByteArray bh = ta.fromHex (ta);
                text.append (codec->toUnicode (bh));
                i += 4;
            }
            else
            {
                text.append (ba.at(i));
                i++;
            }
    }

    strText = rtf_strip (text);

    return true;
}

bool CWizRtfReader::rtf2hmlt(const QString& strRtf, QString& strHtml)
{
    std::string strResult = strRtf.toStdString();
    const char* ch = strResult.c_str();
    rtf2html(ch, strResult);
    strHtml = QString::fromStdString(strResult);

    qDebug() << "rtf 2 html utf8 " << strHtml.toUtf8();
    qDebug() << "rtf 2 html Latin1 " << strHtml.toLatin1();

    return true;
}


