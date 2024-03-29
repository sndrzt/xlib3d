// sudo apt-get install build-essential libglew-dev
// g++ -o cilindro3d cilindro3d.c -lX11 -lGL -lGLEW -L/usr/X11/lib -I/opt/X11/include
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

Display *dis;
Window win;
GC gcontext;
Atom wm_delete_window;
XEvent report;
XWindowAttributes xwatt;
int XRes,YRes,MaxX,MaxY,MinX,MinY;

#define TRUE (1==1)

unsigned long rgb2long(unsigned long r, unsigned long g, unsigned long b)
{
    return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

typedef struct
{
    double x;
    double y;
    double z;
} Ponto3D;

typedef int tri[3];
typedef int rec[4];

typedef struct
{
    Ponto3D *VertArr;
    tri *TriArr;
    rec *RecArr;
    int nVert;
    int ntFaces;
    int nrFaces;

} CylMesh;

typedef struct
{
    Ponto3D ox;
    Ponto3D oy;
    Ponto3D oz;
} View;

int AlocarCilindro(int r, int h, int nFaces, CylMesh *cyl)
{
    int  count = 0;
    int count2 = 0;
    int count3 = 0;
    int i;
    double theta = 2.0*M_PI/(double)nFaces;
    Ponto3D p;

    cyl->nVert =(nFaces+1)*2 + 2;

    // nFaces * 2 (o cilindro tem 2 circulos)
    cyl->ntFaces=nFaces*2;

    //
    cyl->nrFaces =nFaces;

    cyl->TriArr = (tri *)malloc(sizeof(tri) * cyl->ntFaces * 3);
    cyl->RecArr = (rec *)malloc(sizeof(rec) * cyl->nrFaces * 4);
    cyl->VertArr = (Ponto3D *)malloc(sizeof(Ponto3D) * cyl->nVert);

    if((cyl->VertArr == NULL)||(cyl->TriArr == NULL)||(cyl->RecArr == NULL))
        return(0);

    p.x = 0;
    p.y = 0;
    p.z = -h/2.0;
    cyl->VertArr[count++] = p;
    p.x = 0;
    p.y = 0;
    p.z = h/2;
    cyl->VertArr[count++] = p;
    for (i = 0; i <= nFaces; i++)
    {
        //vertices.push_back((vetor){r*cos((double)i*theta), r*sin((double)i*theta), -h/2.0});
        p.x =  r*cos((double)i*theta);
        p.y =  r*sin((double)i*theta);
        p.z = -h/2.0;
        cyl->VertArr[count++] = p;
        //vertices.push_back((vetor){r*cos((double)i*theta), r*sin((double)i*theta), h/2.0});
        p.x =  r*cos((double)i*theta);
        p.y =  r*sin((double)i*theta);
        p.z = h/2;
        cyl->VertArr[count++] = p;

        if(i < nFaces)
        {
            cyl->TriArr[count2][0] = 0;
            cyl->TriArr[count2][1] = i*2+4;
            cyl->TriArr[count2][2] = i*2+2;
            count2 += 1;

            cyl->TriArr[count2][0] = 1;
            cyl->TriArr[count2][1] = i*2+3;
            cyl->TriArr[count2][2] = i*2+5;
            count2 += 1;
            //     afaces.push_back(i*2+2);afaces.push_back(i*2+4);afaces.push_back(i*2+5);afaces.push_back(i*2+3);
            //    aFaces.push_back(afaces);
            //    afaces.clear();
            cyl->RecArr[count3][0] = i*2+2;
            cyl->RecArr[count3][1] = i*2+4;
            cyl->RecArr[count3][2] = i*2+5;
            cyl->RecArr[count3][3] = i*2+3;
            count3 += 1;
        }
    }

    return(1);
}

XPoint Converte(Ponto3D p)
{
    XPoint pp=
    {
        .x = (short int)p.x,
        .y = (short int)p.y
    };
    return pp;
}

void triangle(Ponto3D * vert)
{
    XPoint tr[3];
    tr[0] = Converte(vert[0]);
    tr[1] = Converte(vert[1]);
    tr[2] = Converte(vert[2]);
    XFillPolygon(dis,win,gcontext, tr, 3, Nonconvex,CoordModeOrigin);
}

void rectangle(Ponto3D * vert)
{
    XPoint tr[4];
    tr[0] = Converte(vert[0]);
    tr[1] = Converte(vert[1]);
    tr[2] = Converte(vert[2]);
    tr[3] = Converte(vert[3]);
    XFillPolygon(dis,win,gcontext, tr, 4, Nonconvex,CoordModeOrigin);
}

void putpixel(int x, int y)
{
    XDrawPoint(dis,win,gcontext,x,y);
}

double VecDot(Ponto3D A, Ponto3D B)
{
    return(A.x * B.x + A.y*B.y + A.z*B.z);
}

const View DefCam= { { 0.97,-0.02,0.24 },
    {-0.046,0.97,0.25},
    {-0.24,-0.25,0.94}
};
static View Cam= {   { 0.97,-0.02,0.24 },
    {-0.046,0.97,0.25},
    {-0.24,-0.25,0.94}
};

static double SinX, SinY, CosX, CosY, SinZ, CosZ;

void  SetCam(double ax,double ay, double az)
{
    SinX = sin(ax);
    SinY = sin(ay);
    SinZ = sin(az);
    CosX = cos(ax);
    CosY = cos(ay);
    CosZ = cos(az);

    Cam.ox.x= (CosY * CosX);
    Cam.ox.y= (CosY * SinX);
    Cam.ox.z= SinY;

    Cam.oy.x= (- CosZ * SinX - SinZ * SinY * CosX);
    Cam.oy.y= (CosZ * CosX - SinZ * SinY * SinX);
    Cam.oy.z= (SinZ * CosY);

    Cam.oz.x= (SinZ * SinX - CosZ * SinY * CosX);
    Cam.oz.y= (- SinZ * CosX - CosZ * SinY * SinX);
    Cam.oz.z= (CosZ * CosY);
}

Ponto3D TSR(Ponto3D in, View cam, int dist)
{
    Ponto3D out;
    out.x = VecDot(in,cam.ox);
    out.y = VecDot(in,cam.oy);
    out.z = VecDot(in,cam.oz) + dist;
    return(out);
}

void ProjetaVert(Ponto3D * prj, Ponto3D vert[], int n, double escala)
{
    Ponto3D p;
    Ponto3D out;
    int i;
    int Cx = MaxX >> 1;
    int Cy = MaxY >> 1;
    for(i=0; i < n; i++)
    {
        out = TSR(vert[i],Cam, 100);
        p.x = Cx + escala*out.x * (double)MaxX/((double)MaxX + out.z);
        p.y = Cy - escala*out.y * (double)MaxX/((double)MaxX + out.z);
        p.z = out.z;
        prj[i] = p;
    }
}

Ponto3D VecSub(Ponto3D A, Ponto3D B)
{
    Ponto3D r=
    {
        .x = (A.x - B.x),
        .y = (A.y - B.y),
        .z = (A.z - B.z)
    };
    return(r);
}

Ponto3D VecCross(Ponto3D A, Ponto3D B)
{
    Ponto3D r=
    {
        .x= (A.y*B.z - B.y*A.z),
        .y= (A.z*B.x - B.z*A.x),
        .z= (A.x*B.y - B.x*A.y),
    };
    return(r);
}

double VecLen(Ponto3D A)
{
    return(sqrt(A.x * A.x + A.y * A.y + A.z * A.z));
}

double Sombra(Ponto3D VNormal)
{
    double dir,l=1,len,Cos;
    Ponto3D light = { 30,30,-30};
    const double contraste = 0.6;

    light = TSR(light,DefCam,150);

    len= (VecLen(VNormal)*VecLen(light));
    if(len > 0.001)
        Cos = VecDot(VNormal,light)/len;
    else
        return(l);

    dir = 1.0 - Cos*Cos;

    if(dir >= 0)
        dir = sqrt(dir);

    dir = 1 - contraste*dir;
    l = (dir > 0.0 && dir < 1.0)? dir : 0;

    return l;
}

Ponto3D NormalDaFace(Ponto3D pts[])
{
    Ponto3D N,V1,V2;
    V1 = VecSub(pts[1],pts[0]);
    V2 = VecSub(pts[0],pts[2]);
    N = VecCross(V1,V2);

    return N;
}

static Ponto3D * tr;

void DrawCylMesh(CylMesh cyl)
{
    Ponto3D points[4];
    Ponto3D N;
    double l;
    int i;
    ProjetaVert(tr,cyl.VertArr,cyl.nVert,1.2);

    for(i=0; i < cyl.ntFaces; i++)
    {
        points[0] =tr[cyl.TriArr[i][0]];
        points[1] =tr[cyl.TriArr[i][1]];
        points[2] =tr[cyl.TriArr[i][2]];
        N = NormalDaFace(points);

        if(N.z < 0.0)
        {
            l = Sombra(N);
            XSetForeground(dis,gcontext,rgb2long(255*l,45*l,120*l));
            triangle(points);
        }
    }

    for(i=0; i < cyl.nrFaces; i++)
    {
        points[0] =tr[cyl.RecArr[i][0]];
        points[1] =tr[cyl.RecArr[i][1]];
        points[2] =tr[cyl.RecArr[i][2]];
        points[3] =tr[cyl.RecArr[i][3]];
        N = NormalDaFace(points);

        if(N.z < 0.0)
        {
            l = Sombra(N);
            XSetForeground(dis,gcontext,rgb2long(255*l,45*l,120*l));
            rectangle(points);
        }
    }
}

CylMesh cylinder;

void closegraph()
{
    XCloseDisplay(dis);
    free(cylinder.TriArr);
    free(cylinder.RecArr);
    free(cylinder.VertArr);
    free(tr);
    printf("Bye bye...");
}

int main()
{
    XRes = 600;
    YRes = 600;
    int Raio = (int)(2.2*(double)(YRes-20)/7.0);
    int nTriangulos = 32;
    int canto;
    char msg[]="Use as teclas de seta para girar o cilindro";
    double angulo = 15;
    if(AlocarCilindro(80,180,nTriangulos,&cylinder))
    {

        tr = (Ponto3D *)malloc(sizeof(Ponto3D) * cylinder.nVert);
        if(tr == NULL)
        {
            printf("Erro de alocacao de vertices");
            return(1);
        }
    }
    else
    {

        printf("nao foi possível alocar esfera");
        return(2);

    }
    if(NULL != (dis = XOpenDisplay(NULL)))
    {
        win = XCreateSimpleWindow(dis,RootWindow(dis, 0),1,1,XRes,YRes,1,
                                  BlackPixel(dis,0), WhitePixel(dis,0));
        gcontext = XCreateGC(dis,win,0,0);
        XSelectInput(dis, win, StructureNotifyMask | ExposureMask | KeyPressMask | ButtonPressMask);
        wm_delete_window =XInternAtom (dis, "WM_DELETE_WINDOW", False);
        XSetWMProtocols (dis,win, &wm_delete_window, 1);
        XGetWindowAttributes(dis, win, &xwatt);
        MaxX = xwatt.width - 1;
        MaxY = xwatt.height - 1;
        MinX = xwatt.x;
        MinY = xwatt.y;
        XFlush(dis);
        XMapWindow(dis, win);
        XFlush(dis);
    }
    else
    {
        exit(1);
    }

    SetCam(0,0,angulo);
    atexit(closegraph);
    while(TRUE)
    {
        if(XPending(dis) > 0)
        {
            XNextEvent(dis,&report);
            switch(report.type)
            {
            case KeyPress:
                switch(XLookupKeysym(&report.xkey, 0))
                {
                case XK_q:
                case XK_Q:

                    return(0);
                    break;
                case XK_Up:
                case XK_Right:
                    angulo += 0.03;
                    SetCam(0,0,angulo);
                    break;
                case XK_Down:
                case XK_Left:
                    angulo -= 0.03;
                    SetCam(0,0,angulo);
                    break;
                }
            case Expose:
                XDrawString(dis,win,gcontext,20,20,msg,strlen(msg));
                XSetForeground(dis,gcontext,0);
                canto = Raio * sqrt(2);
                XFillRectangle(dis,win,gcontext,XRes/2 - canto,YRes/2 - canto,2*canto,2*canto);
                DrawCylMesh(cylinder);

                break;
            case ClientMessage:
                if((Atom)report.xclient.data.l[0] == wm_delete_window)
                    return(0);

                break;

            }
        }
        XFlush(dis);
    }

    return(0);
}

