#include<iostream>
#include<SFML/Graphics.hpp>
#include<vector>
#include<math.h>
#define PI 3.14159265

using namespace std;
using namespace sf;
Vector2i windowSize(600,400);
Vector2i mapSize(16,15);
Vector2f boxSize(windowSize.x/8.f,windowSize.y/8.f);
vector<Vertex> mapVertices;
Texture wallTex,floorTex,ceilTex;
float wallHeight=windowSize.y/3.f;
int Tmap[]=
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,1,
    1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,1,1,1,1,1,0,0,0,1,
    1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,
    1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,
    1,0,0,0,0,0,0,1,1,1,1,1,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
};
void drawMap()
{
    Vertex temp;
    for (int i = 0; i < mapSize.y; i++)
    {
        for (int j = 0; j < mapSize.x; j++)
        {
            temp.color=Tmap[mapSize.x*i+j]==1?Color::White:Color::Black;
            temp.position=Vector2f(j*boxSize.x+1,i*boxSize.y+1);                
            mapVertices.push_back(temp);
            temp.position=Vector2f((j+1)*boxSize.x-1,(i)*boxSize.y+1);                
            mapVertices.push_back(temp);
            temp.position=Vector2f((j+1)*boxSize.x-1,(i+1)*boxSize.y-1);                
            mapVertices.push_back(temp);
            temp.position=Vector2f((j)*boxSize.x+1,(i+1)*boxSize.y-1);                
            mapVertices.push_back(temp);

        }   
    }
}

vector<Vertex> wallPoints,floorPoints,ceilPoints;
class Player
{
private:
    Vector2f position;
    float viewAngle,FOV,upAngle;
    float height;
public:
    int state;  //crouch/stand/jumps
    CircleShape body;
    vector<Vertex> ray;
    Player(Vector2f position)
    {
        this->position=position;
        height=wallHeight/2.f;
        upAngle=0.f;
        body.setRadius(5.f);
        body.setOrigin(5.f,5.f);
        body.setPosition(position);
        body.setFillColor(Color::Yellow);
        viewAngle=0.f;
        FOV=60; //in degrees
        initializeRays();

    }
    void lookUpDown(float thetaUp)
    {
        if (upAngle+thetaUp<windowSize.y/2.f && upAngle+thetaUp>-windowSize.y/2.f)  //theta is offset to move projection plane
        {
            upAngle+=thetaUp;   //in degree
        }
    }

    void updateView(int frward,int side,float theta)
    {
        float viewOffset=side*side*90.f*PI/180.f;   //side=+-1
        viewAngle+=theta;
        float dx=frward*cos(viewAngle)+side*cos(viewAngle+viewOffset);
        float dy=frward*sin(viewAngle)+side*sin(viewAngle+viewOffset);
        dx/=frward*side!=0?sqrt(2+2*cos(viewOffset)):1;
        dy/=frward*side!=0?sqrt(2+2*cos(viewOffset)):1;
        dx*=4.f;    //speed
        dy*=4.f;      //speed
        height=state==-1?wallHeight/4.f:wallHeight/2.f;


        if(!checkCollision(position.x+0.1*boxSize.x*dx,position.y+0.1*boxSize.x*dy))
        {
            position.x+=dx;
            position.y+=dy;
            body.setPosition(position);
            initializeRays();
        }
       
    }
    
    bool checkCollision(float x,float y)
    {
        Vector2i index((int)(x/boxSize.x),(int)(y/boxSize.y));
        if (Tmap[index.y*mapSize.x+index.x]==1)
        {
            return true;
        }
        else
        {
            return false;
        }
        
    }
    void initializeRays()
    {
        ray.clear();
        wallPoints.clear();
        floorPoints.clear();
        ceilPoints.clear();
        float dTheta=FOV/windowSize.x; //in degrees
        for (int i = 0; i < (int)(FOV/dTheta); i++)
        {
            createRays(dTheta*i-FOV/2.f,dTheta);
        }
    }
    int createRays(float thetaOff,int dTheta)
    {
        float Hdist=100.f*windowSize.x,Vdist=100.f*windowSize.y;
        float viewDir=viewAngle+thetaOff*PI/180.f;
        Vector2i boxIndex((int)(position.x/boxSize.x),(int)(position.y/boxSize.y));
        int checkDir=sin(viewDir)>=0?1:-1,n=1;
        float xOff,yOff;
        while (boxIndex.y+n*checkDir<mapSize.y && boxIndex.y+n*checkDir>=0)
        {
            float yOnLine=checkDir==1?boxSize.y*boxIndex.y+n*boxSize.y:boxSize.y*boxIndex.y-(n-1)*boxSize.y;
            yOff=yOnLine-position.y;
            xOff=tan(viewDir)==0.f?(yOff)/tan(viewDir+0.001f):(yOff)/tan(viewDir);
            Vector2i checkIndex((int)((position.x+xOff)/boxSize.x),(int)((yOnLine-0.1*boxSize.y)/boxSize.y));  //0.1*boxSize is offset
            if (checkIndex.x>=mapSize.x || checkIndex.x<0)
            {
                break;
            }
            //checkIndex.x=checkIndex.x>=mapSize.x?mapSize.x-1:checkIndex.x;
            //checkIndex.x=checkIndex.x<0?0:checkIndex.x;
            if (Tmap[mapSize.x*checkIndex.y+checkIndex.x]==1 || Tmap[mapSize.x*checkIndex.y+checkIndex.x+mapSize.x]==1)
            {
                Hdist=sqrt(xOff*xOff+yOff*yOff);
                break;
            }
            n++;
        }
        Vector2f Hdata(xOff,yOff);
        checkDir=cos(viewDir)>=0?1:-1;
        n=1;
        while (boxIndex.x+n*checkDir<mapSize.x && boxIndex.x+n*checkDir>=0)
        {
            float xOnLine=checkDir==1?boxSize.x*boxIndex.x+n*boxSize.x:boxSize.x*boxIndex.x-(n-1)*boxSize.x;
            xOff=xOnLine-position.x;
            yOff=(xOff)*tan(viewDir);
            Vector2i checkIndex((int)((xOnLine-0.1*boxSize.x)/boxSize.x),(int)((position.y+yOff)/boxSize.y));  //0.1*boxSize is offset
            if (checkIndex.y>=mapSize.y || checkIndex.y<0)
            {
                break;
            }
            
           // checkIndex.y=checkIndex.y>=mapSize.y?mapSize.y-1:checkIndex.y;
           // checkIndex.y=checkIndex.y<0?0:checkIndex.y;
            if (Tmap[mapSize.x*checkIndex.y+checkIndex.x]==1 || Tmap[mapSize.x*checkIndex.y+checkIndex.x+1]==1)
            {
                Vdist=sqrt(xOff*xOff+yOff*yOff);
                break;
            }
            n++;
        }
        Vector2f Vdata(xOff,yOff);
        float d=Hdist<=Vdist?Hdist:Vdist;
        
      /*  if (d>=2*windowSize.x)   //cutoff distance
        {
            return 0;
        }*/
        

        Vertex temp;
        temp.color=Color::Green;
        temp.position=position;
        ray.push_back(temp);
        temp.position=Hdist<=Vdist?position+Hdata:position+Vdata;
        ray.push_back(temp);

        //texture:
        Vector2u Tsize=wallTex.getSize();
        int texCoord=Hdist<=Vdist?(int)(temp.position.x)%Tsize.x:(int)(temp.position.y)%Tsize.x;      //temp currently has endpoint of ray
        //projector:
        float dProjectionPlane=(0.5*windowSize.x)/tan(FOV*PI/360.f);
        float xp=(thetaOff+FOV/2.f)*windowSize.x/(FOV-dTheta); //each ray defines a column
        float thetaOffRadians=thetaOff*PI/180.f;
        float dStraight=d*cos(thetaOffRadians);
        float yp=(wallHeight-height)*dProjectionPlane/dStraight;
        float dmin=(0.5*wallHeight)*dProjectionPlane/(0.5f*windowSize.y);  //0.5*wallheight is a reference
        float torchEffect=cos(thetaOff/(0.5*FOV)*PI/2.f);
        float colorScale=d<=dmin?250*torchEffect:250.f*dmin/d*torchEffect;    //for color 
        //draw wall slice://
        temp.color=Color(colorScale,colorScale,colorScale);  
        temp.texCoords=Vector2f(texCoord,0.f);
        temp.position.x=xp;
        temp.position.y=0.5*windowSize.y+upAngle-yp;    //move projection plane to look upDown
        wallPoints.push_back(temp);
        //yp updated for bottom point of wall:
        yp=height*dProjectionPlane/(dStraight); //use this yp to draw floor
        temp.texCoords=Vector2f(texCoord,Tsize.y);
        temp.position.y=0.5*windowSize.y+upAngle+yp;
        wallPoints.push_back(temp);

        //draw floor slice://
        if (yp+upAngle<windowSize.y/2.f)
        {
            int NfloorRays=(int)(windowSize.y/2.f-yp-upAngle);
            Vector2u Fsize=floorTex.getSize();
            dmin=dProjectionPlane*(0.5*wallHeight)/(0.5*windowSize.y);
            for (int i = 0; i <= NfloorRays; i++)   //drawn pointwise
            {
                temp.position.y=0.5*windowSize.y+yp+upAngle+i;
                float dFloor=dProjectionPlane*height/(yp+(float)i);
                dFloor/=cos(thetaOffRadians);
                float xFloor=position.x+dFloor*cos(viewDir);
                float yFloor=position.y+dFloor*sin(viewDir);
                temp.texCoords=Vector2f((int)xFloor%Fsize.x,(int)yFloor%Fsize.y);  
                colorScale=dFloor<dmin?250.f*torchEffect:250.f*dmin/dFloor*torchEffect;
                temp.color=Color(colorScale,colorScale,colorScale);  
                floorPoints.push_back(temp);
            }
        }
        //draw ceiling slice://
        yp=(wallHeight-height)*dProjectionPlane/dStraight;  //use old yp
        if (yp-upAngle<windowSize.y/2.f)
        {
            int NCeilRays=(int)(windowSize.y/2.f-yp+upAngle);
            Vector2u Ceilsize=ceilTex.getSize();
            dmin=dProjectionPlane*(0.5*wallHeight)/(0.5*windowSize.y);
            for (int i = 0; i <= NCeilRays; i++)   //drawn pointwise
            {
                temp.position.y=0.5*windowSize.y+upAngle-yp-i;
                float dCeil=dProjectionPlane*(wallHeight-height)/(yp+(float)i);
                dCeil/=cos(thetaOffRadians);
                float xFloor=position.x+dCeil*cos(viewDir);
                float yFloor=position.y+dCeil*sin(viewDir);
                temp.texCoords=Vector2f((int)xFloor%Ceilsize.x,(int)yFloor%Ceilsize.y);  
                colorScale=dCeil<dmin?250.f*torchEffect:250.f*dmin/dCeil*torchEffect;
                temp.color=Color(colorScale,colorScale,colorScale);  
                ceilPoints.push_back(temp);
            }
        }
        
        return 1;

    }

    Vector2f getPos()
    {
        return position;
    }
};



int main()
{
    RenderWindow w(VideoMode(800,600),"raytrace");
    RenderTexture buffer;
    buffer.create(windowSize.x,windowSize.y);
    Sprite canvas;
    w.setFramerateLimit(60.f);
    if(!wallTex.loadFromFile("./rayCast/t5.jpeg",IntRect(0,0,wallHeight,wallHeight)))
    {
        cout<<"Failed to load wall texture"<<endl;
    }
    if(!floorTex.loadFromFile("./rayCast/t6.jpeg"))
    {
        cout<<"Failed to load floor texture"<<endl;
    }
    if(!ceilTex.loadFromFile("./rayCast/t7.jpeg"))
    {
        cout<<"Failed to load ceiling texture"<<endl;
    }
    Texture gunTex,gunReload1Tex,gunReload2Tex;
    if(!gunTex.loadFromFile("./rayCast/gun1.png"))
    {
        cout<<"Failed to load gun texture"<<endl;
    }
    if(!gunReload1Tex.loadFromFile("./rayCast/gunR1.png"))
    {
        cout<<"Failed to load gun texture"<<endl;
    }
    if(!gunReload2Tex.loadFromFile("./rayCast/gunR2.png"))
    {
        cout<<"Failed to load gun texture"<<endl;
    }
    Sprite gun;
    Vector2u gunSize=gunTex.getSize();
    gun.setPosition((windowSize.x-gunSize.x/4.f)/2.f,windowSize.y-gunSize.y);
    gun.setTexture(gunTex);
    gun.setTextureRect(sf::IntRect((int)(3.f*gunSize.x/4.f),0,(int)(gunSize.x/4.f),gunSize.y));
    drawMap();
    Player p1(Vector2f(100.f,100.f));
    bool processMoves=false,menuToggle=false,shoot=false,reload=false;
    int moveForward=0,moveSide=0,rotateView=0,shootIndex=0,reloadIndex=0,actionTime=0;
    
    Mouse::setPosition(Vector2i(windowSize.x/2,windowSize.y/2),w);
    View mainView(Vector2f(windowSize.x/2.f,windowSize.y/2.f),(Vector2f)windowSize);
    float miniMapX=mapSize.x*boxSize.x,miniMapY=mapSize.y*boxSize.y;
    View miniMap(Vector2f(miniMapX/2.f,miniMapY/2.f),Vector2f(miniMapX,miniMapY));
    mainView.setViewport(sf::FloatRect(0.f,0.f,1.f,1.f));
    miniMap.setViewport(sf::FloatRect(0.8f,0.8f,0.2f,0.2f));
    w.setMouseCursorVisible(false);
    
    while (w.isOpen())
    {
        
        Event e;
        while (w.pollEvent(e))
        {
            if (e.type==Event::Closed)
            {
                w.close();
            }
            if (e.type==Event::KeyPressed)
            {
                switch (e.key.code)
                {
                case Keyboard::R:
                    reloadIndex=!reload?0:reloadIndex;
                    actionTime=!reload?1:actionTime;
                  
                    reload=true;
                    gunSize=gunReload1Tex.getSize();
                    gun.setTexture(gunReload1Tex);
                    shoot=false;
                    break;
                case Keyboard::C:
                    p1.state=p1.state==-1?0:-1;    //0=stand,-1=crouch,1=jump
                    processMoves=true;
                    break;
                case Keyboard::Escape:
                    menuToggle=menuToggle?false:true;
                    if (menuToggle)
                    {
                        w.setMouseCursorVisible(true);
                    }
                    else
                    {
                        w.setMouseCursorVisible(false);
                    }
                    
                    
                    break;
                }
            }
            if (e.type==Event::MouseButtonPressed)
            {
                if (!menuToggle && Mouse::isButtonPressed(Mouse::Left) && !reload)
                {
                    shootIndex=!shoot?2:shootIndex;
                    actionTime=!shoot?1:actionTime;
                    shoot=true;      
                }
                
            }
                
        }

        
        if (Keyboard::isKeyPressed(Keyboard::W))
        {
            moveForward+=1;
            processMoves=true;
        }
        if (Keyboard::isKeyPressed(Keyboard::S))
        {
            moveForward+=-1;
            processMoves=true;
        }
        if (Keyboard::isKeyPressed(Keyboard::A))
        {
            moveSide+=-1;
            processMoves=true;
            //p1.updatePos(-1,90);    //offset 90 degrees to move left/right
        }
        if (Keyboard::isKeyPressed(Keyboard::D))
        {
            moveSide+=1;
            processMoves=true;
            //p1.updatePos(1,90);
        }
        if (!menuToggle)
        {
            Vector2i mousePos=Mouse::getPosition(w);
            if (mousePos.x!=(int)windowSize.x/2 || (int)windowSize.y/2)
            {
                rotateView=0.1f*(mousePos.x-windowSize.x/2);
                p1.lookUpDown(windowSize.y/2-mousePos.y);
                processMoves=true;
                Mouse::setPosition(Vector2i(windowSize.x/2,windowSize.y/2),w);
            }
            if (shoot && actionTime%4==0)
            {
                if (shootIndex<0)
                {
                    shoot=false;
                    shootIndex=3;
                    actionTime=0;
                }
                else
                {
                    shootIndex-=1;
                }
                gun.setTextureRect(sf::IntRect((int)(shootIndex*gunSize.x/4.f),0,(int)(gunSize.x/4.f),gunSize.y));
            }
            else if (reload && actionTime%6==0)
            {
                if (reloadIndex<=1)
                {
                    gun.setTextureRect(sf::IntRect((int)(reloadIndex*gunSize.x/2.f),0,(int)(gunSize.x/2.f),gunSize.y));
                }
                else if (reloadIndex>1 && reloadIndex<=3)
                {
                    gun.setTexture(gunReload2Tex);
                    gun.setTextureRect(sf::IntRect((int)((reloadIndex-2)*gunSize.x/2.f),0,(int)(gunSize.x/2.f),gunSize.y));    
                }
                else if (reloadIndex>3)
                {
                    reload=false;
                    gun.setTexture(gunTex);
                    gunSize=gunTex.getSize();
                    gun.setTextureRect(sf::IntRect((int)(3.f*gunSize.x/4.f),0,(int)(gunSize.x/4.f),gunSize.y));    
                    actionTime=0;
                }
                reloadIndex+=1;
                
            }
            
            
        }
        if (processMoves)
        {
            processMoves=false;
            p1.updateView(moveForward,moveSide,(float)rotateView*PI/180.f);
            moveForward=0;
            moveSide=0;
            rotateView=0;
        }
        
        actionTime+=actionTime!=0?1:0;
        
        buffer.clear();
        buffer.setView(mainView);
      //  if (toggle)
        {
            if (!ceilPoints.empty())
            {    
                buffer.draw(ceilPoints.data(),ceilPoints.size(),Points,&ceilTex);
            }
            
            if (!floorPoints.empty())
            {    
                buffer.draw(floorPoints.data(),floorPoints.size(),Points,&floorTex);
            }
            
            buffer.draw(wallPoints.data(),wallPoints.size(),Lines,&wallTex);
            buffer.draw(gun);
        }
        buffer.setView(miniMap);
       // else
        {
            buffer.draw(mapVertices.data(),mapVertices.size(),Quads);
           // buffer.draw(p1.ray.data(),p1.ray.size(),Lines);
            buffer.draw(p1.body);
        }
        
        buffer.display();   
        canvas.setTexture(buffer.getTexture());
        canvas.setScale(w.getSize().x/(float)windowSize.x,w.getSize().y/(float)windowSize.y);
        w.clear();
        w.draw(canvas);
        w.display();


    
    }
    return 0;
}