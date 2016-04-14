#include "detection.h"

/****************************************************************************
** Detects edges in the specified QImage
****************************************************************************/
QImage edges(const QImage &source)
{
  /* initialisation */
  QImage binary = QImage(source.size(), QImage::Format_Mono);

  /*** Sobel edge detection ***/

  /* Set up Lx, Ly */
  QVector<QByteArray> Lx(3), Ly(3);

  Lx[0][0] = -1;  Lx[0][1] = +0;  Lx[0][2] = +1;
  Lx[1][0] = -2;  Lx[1][1] = +0;  Lx[1][2] = +2;
  Lx[2][0] = -1;  Lx[2][1] = +0;  Lx[2][2] = +1;

  Ly[0][0] = +1;  Ly[0][1] = +2;  Ly[0][2] = +1;
  Ly[1][0] = +0;  Ly[1][1] = +0;  Ly[1][2] = +0;
  Ly[2][0] = -1;  Ly[2][1] = -2;  Ly[2][2] = -1;

  for(unsigned int x = 0; x < source.width(); x++)
  {
    for(unsigned int y = 0; y < source.height(); y++)
    {
      double new_x = 0.0, new_y = 0.0;

      /* gradient */
      for(int i = -1; i <= 1; i++)
      {
        for(int j = -1; j <= 1; j++)
        {
          /* these are offset co-ords */
          int _x = x + i;
          int _y = y + j;

          /* bounds checking */
          if (_x < 0)                     _x = -_x;
          else if (_x >= source.width())  _x = 2 * source.width() - _x - 2;

          if (_y < 0)                     _y = -_y;
          else if (_y >= source.height()) _y = 2 * source.height() - _y - 2;

          /* accumulate */
          int gray = qGray(source.pixel(_x, _y));
          new_x += Lx[i + 1][j + 1] * gray;
          new_y += Ly[i + 1][j + 1] * gray;
        }
      }

      /* using 128 as a threshold, decide if the steepness is sufficient (= edge = 1) */
      int pixel = sqrt(pow(new_x, 2) + pow(new_y, 2)) > 128 ? 1 : 0;
      binary.setPixel(x, y, pixel);
    }
  }

  return binary;
}

QVector<QPoint> hough(const QImage &image)
{
    QVector<QPoint> res;
    QRgb BLANC = qRgb(255,255,255);

    int max_rho = sqrt(image.width()*image.width()+image.height()*image.height());
    int max_theta = 360;
    int pho=0, angle = 0;

    //Initialisation de l'accumulateur
    int** accumulateur = new int*[max_rho];
    for(int i=0;i<max_rho;i++) accumulateur[i] = new int[max_theta];
    for(int i=0;i<max_rho;i++)
        for(int j=0;j<max_theta;j++)
            accumulateur[i][j] = 0;

    //Pour chaque pixel
    for(int x=0;x<image.height();x++)
    {
        for(int y=0;y<image.width();y++)
        {
            //Si le pixel est blanc
            if(image.pixel(y,x) == BLANC)
            {
                for(int i=0;i<max_theta;i++)
                {
                    pho = y*cos(i*M_PI/180)+x*sin(i*M_PI/180);
                    //On modifie l'angle si pho < 0
                    angle = i;
                    if(pho<0)
                    {
                        pho*=-1;
                        if(i>=180) angle = i-180;
                        else angle = i+180;
                    }
                    //On met dans l'accumulateur
                    accumulateur[pho][angle]++;
                }
            }
        }
    }

    //On sauve les valeurs maximals (plus de 10 points alignés)
    for(int i=0;i<max_rho;i++)
    {
        for(int j=0;j<max_theta;j++)
        {
            if(accumulateur[i][j]>=30) res.push_back(QPoint(i,j));
        }
    }

    //On supprime l'accumulateur
    for(int i=0;i<max_rho;i++) delete[] accumulateur[i];
    delete[] accumulateur;

    return res;
}

void avoirLigneAngle(const QVector<QPoint> &ligne, QVector<QPoint> &temp, int angle, int ecart)
{
    for(int i=0; i<ligne.size();i++)
    {
        //Si la droite à le bon angle
        if(ligne[i].y() <= (angle+ecart) && ligne[i].y() >= (angle-ecart))
        {
            temp.push_back(ligne[i]);
        }
    }

}

void tracerLigne(QImage& image, int rho, int theta, QRgb couleur)
{
    //2 points de la droite a tracer
    QPoint p1, p2;

    p1.setX(rho*cos(theta*M_PI/180));
    p1.setY(rho*sin(theta*M_PI/180));

    p2.setX(-p1.y()+p1.x());
    p2.setY(p1.x()+p1.y());

    tracerDroite(image,p1,p2,couleur);
}

void tracerDroite(QImage& image, QPoint p1, QPoint p2, QRgb couleur)
{
    double a=0.0;
    double b=0.0;

    if(p2.x()-p1.x() !=0)
    {
        a = ((double)p2.y()-(double)p1.y())/((double)p2.x()-(double)p1.x());
        b = p1.y() - a*p1.x();
        if (a>1.0 || a<-1.0)
        {
             for(int i=0;i<image.height();i++)
                 if((i-b)/a > 0 && (i-b)/a < image.width())
                     image.setPixel((i-b)/a,i,couleur);
        }
        else
        {
            for(int i=0;i<image.width();i++)
                if(a*i+b > 0 && a*i+b < image.height())
                    image.setPixel(i,a*i+b,couleur);
        }
    }
    else
    {
        for(int i=0;i<image.height();i++)
            image.setPixel(p1.x(),i,couleur);
    }

}
