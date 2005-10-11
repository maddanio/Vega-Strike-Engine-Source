#include "from_obj.h"
#include <map>
#include <set>
#include <vector>
#include <assert.h>
using std::map;
using std::vector;
struct MTL:public GFXMaterial {
  MTL() {
    blend_src=ONE;
    blend_dst=ZERO;
    reflect=true;
  }  
  bool usenormals;
  bool reflect;
   int blend_src;int blend_dst;
   vector<textureholder> textures;
   textureholder detail;   
  
   vector <float> detailplanei;
   vector <float> detailplanej;
   vector <float> detailplanek;
};
struct IntRef {
  int v;
  int n;
  int t;
  IntRef() {v=-1;n=-1;t=-1;}
  IntRef(int vv, int nn, int tt) {v=vv;n=nn;t=tt;}

  bool operator==(const IntRef& o) const { return (v==o.v && n==o.n && t==o.t); };
  bool operator!=(const IntRef& o) const { return (v!=o.v || n!=o.n || t!=o.t); };
  bool operator< (const IntRef& o) const { 
      if (v<o.v) return true; else if (v==o.v) {
          if (n<o.n) 
              return true; else if (n==o.n)
              return (t<o.t);
      }
      return false;
  };
};
struct VTX {
  VTX(float _x, float _y, float _z) : x(_x),y(_y),z(_z) {};
  VTX() : x(0),y(0),z(0) {};

  bool operator==(const VTX& o) const { return (x==o.x && y==o.y && z==o.z); };
  bool operator!=(const VTX& o) const { return (x!=o.x || y!=o.y || z!=o.z); };

  float x,y,z;
};
struct TEX {
  TEX(float _s, float _t) : s(_s),t(_t) {};
  TEX() : s(0),t(0) {};

  bool operator==(const TEX& o) const { return (s==o.s && t==o.t); };
  bool operator!=(const TEX& o) const { return (s!=o.s || t!=o.t); };
  bool operator< (const TEX& o) const { return (s<o.s)||((s==o.s)&&(t<o.t)); };

  float s,t;
};
struct NORMAL {
  NORMAL(float _i, float _j, float _k) : i(_i),j(_j),k(_k) {};
  NORMAL() : i(1),j(0),k(0) {};

  bool operator==(const NORMAL& o) const { return (i==o.i && j==o.j && k==o.k); };
  bool operator!=(const NORMAL& o) const { return (i!=o.i || j!=o.j || k!=o.k); };
  bool operator< (const NORMAL& o) const { return (i<o.i)||((i==o.i)&&((j<o.j)||((j==o.j)&&(k<o.k)))); };

  float i,j,k;
};
struct FACE {
  FACE() : num(0) {}
  FACE(const IntRef &_r1, const IntRef &_r2) : num(2),r1(_r1),r2(_r2) {};
  FACE(const IntRef &_r1, const IntRef &_r2, const IntRef &_r3) : num(3),r1(_r1),r2(_r2),r3(_r3) {};
  FACE(const IntRef &_r1, const IntRef &_r2, const IntRef &_r3, const IntRef &_r4) : num(4),r1(_r1),r2(_r2),r3(_r3),r4(_r4) {};

  int num;
  IntRef r1,r2,r3,r4;
};

// For stripifying, but not worth it now that we're switching to OGRE...
struct TRISORT : public binary_function<FACE,FACE,bool> {
    bool operator()(const FACE& a, const FACE& b) {
        assert(a.num==3 && b.num==3);
        if (a.r1<b.r1) 
            return true; else if (a.r1==b.r1)
            return (a.r2<b.r2); else
            return false;
    }
};

struct QUADSORT : public binary_function<FACE,FACE,bool> {
    bool operator()(const FACE& a, const FACE& b) {
        assert(a.num==4 && b.num==4);
        if (a.r1<b.r1) 
            return true; else if (a.r1==b.r1)
            return (a.r2<b.r2); else
            return false;
    }
};

vector<string> splitWhiteSpace(string inp) {
  int where;
  vector<string> ret;
  while ((where=inp.find_first_of("\t "))!=string::npos) {
    if (where!=0)
      ret.push_back(inp.substr(0,where));
    inp = inp.substr(where+1);
  }
  if (inp.length()&&where==string::npos)
    ret.push_back(inp);
  return ret;
}
IntRef parsePoly(string inp) {
  IntRef ret;
  const char * st=inp.c_str();
  if (3==sscanf(st,"%d/%d/%d",&ret.v,&ret.t,&ret.n)) {
    ret.v-=1;ret.t-=1;ret.n-=1;
    return ret;
  }
  if (2==sscanf(st,"%d/%d/",&ret.v,&ret.t)) {
    ret.v-=1;ret.t-=1;
    return ret;
  }
  if (2==sscanf(st,"%d/%d",&ret.v,&ret.t)) {
    ret.v-=1;ret.t-=1;
    return ret;
  }
  if (2==sscanf(st,"%d//%d",&ret.v,&ret.n)) {
    ret.v-=1;ret.n-=1;
    return ret;
  }
  sscanf(st,"%d",&ret.v);
    ret.v-=1;
  return ret;  
}
void charstoupper(char *buf){
   while(*buf) {
      *buf=toupper(*buf);
      ++buf;
   }
}
void wordtoupper(char *buf){
   while(*buf&&!isspace(*buf)) {
      *buf=toupper(*buf);
      ++buf;
   }
}
textureholder makeTextureHolder(std::string str, int which) {
  textureholder ret;
  ret.type=TEXTURE;
  if (str.find(".ani")!=string::npos) {
    ret.type=ANIMATION;
  }

  ret.index=which;
  for (std::string::iterator it=str.begin(); it!=str.end(); it++)
      ret.name.push_back((unsigned char)*it);
  return ret;
}

static int AddElement(XML &xml, const IntRef &r, map<IntRef,int> &elements, const vector <VTX> &vtxlist, const vector <TEX> &txclist, const vector <NORMAL> &normallist)
{
    map<IntRef,int>::iterator e = elements.find(r);
    if (e==elements.end()) {
        pair<IntRef,int> elem = pair<IntRef,int>(r,xml.vertices.size());
        elements.insert(elem);
        GFXVertex v;
        if (r.n>=0) {
            v.i = normallist[r.n].i;
            v.j = normallist[r.n].j;
            v.k = normallist[r.n].k;
        } else v.i=v.j=v.k=0;
        if (r.t>=0) {
            v.s = txclist[r.t].s;
            v.t = txclist[r.t].t;
        } else v.s=v.t=0;
        if (r.v>=0) {
            v.x = vtxlist[r.v].x;
            v.y = vtxlist[r.v].y;
            v.z = vtxlist[r.v].z;
        } else v.x=v.y=v.z=0;
        xml.vertices.push_back(v);

        return elem.second;
    } else 
        return (*e).second;
}

static void ObjToXML(XML &xml, const vector <VTX> &vtxlist, const vector <TEX> &txclist, const vector <NORMAL> &normallist, const vector <FACE> &facelist)
{
    map<IntRef,int> elements;

    for (vector<FACE>::const_iterator fiter = facelist.begin(); fiter!=facelist.end(); fiter++) {
        int e1=(((*fiter).num>=1)?AddElement(xml,(*fiter).r1,elements,vtxlist,txclist,normallist):-1);
        int e2=(((*fiter).num>=2)?AddElement(xml,(*fiter).r2,elements,vtxlist,txclist,normallist):-1);
        int e3=(((*fiter).num>=3)?AddElement(xml,(*fiter).r3,elements,vtxlist,txclist,normallist):-1);
        int e4=(((*fiter).num>=4)?AddElement(xml,(*fiter).r4,elements,vtxlist,txclist,normallist):-1);
        switch ((*fiter).num) {
        case 2: 
            //Line...
            xml.lines.push_back(line(e1,e2,xml.vertices[e1].s,xml.vertices[e1].t,xml.vertices[e2].s,xml.vertices[e2].t));
            break;
        case 3: 
            //Triangle...
            xml.tris.push_back(triangle(e1,e2,e3,xml.vertices[e1].s,xml.vertices[e1].t,xml.vertices[e2].s,xml.vertices[e2].t,xml.vertices[e3].s,xml.vertices[e3].t));
            break;
        case 4: 
            //Quad...
            xml.quads.push_back(quad(e1,e2,e3,e4,xml.vertices[e1].s,xml.vertices[e1].t,xml.vertices[e2].s,xml.vertices[e2].t,xml.vertices[e3].s,xml.vertices[e3].t,xml.vertices[e4].s,xml.vertices[e4].t));
            break;
        }
    }
}

static FACE map_face(const FACE& f,const vector<int> &txcmap,const vector<int> &normalmap)
{
    FACE rf(f);
    switch (f.num) {
    case 4: rf.r4.t=txcmap[rf.r4.t];
            rf.r4.n=normalmap[rf.r4.n];
    case 3: rf.r3.t=txcmap[rf.r3.t];
            rf.r3.n=normalmap[rf.r3.n];
    case 2: rf.r2.t=txcmap[rf.r2.t];
            rf.r2.n=normalmap[rf.r2.n];
    case 1: rf.r1.t=txcmap[rf.r1.t];
            rf.r1.n=normalmap[rf.r1.n];
    }
    return rf;
}

string ObjGetMtl (FILE* obj, string objpath) {
    fseek (obj,0,SEEK_END);
    int osize=ftell(obj);
    fseek (obj,0,SEEK_SET);
    char * buf = (char*)malloc((osize+1)*sizeof(char));
    char * str = (char*)malloc((osize+1)*sizeof(char));

    string ret;

    while (ret.empty() && fgets(buf,osize,obj)) {
        if (buf[0]=='#'||buf[0]==0)
            continue;
        if (1==sscanf(buf,"mtllib %s",str))
            ret=str;
    }

    free (buf);
    free (str);

    fseek (obj,0,SEEK_SET);

    //Now, copy path part
    string::size_type p = objpath.rfind('/');
#if defined(_WIN32) && !defined(__CYGWIN__)
    if (p==string::npos) {
        p = objpath.rfind('\\');
    } else {
        string::size_type p2 = objpath.find('\\',p+1);
        if (p2!=string::npos) p=p2;
    }
#endif
    if (p!=string::npos)
        ret = objpath.substr(0,p+1) + ret;

    return ret;
}

void ObjToBFXM (FILE* obj, FILE * mtl, FILE * outputFile,bool forcenormals) {
   fseek (obj,0,SEEK_END);
   int osize=ftell(obj);
   fseek (obj,0,SEEK_SET);
   fseek (mtl,0,SEEK_END);
   {
      int msize=ftell(mtl);
      osize = osize>msize?osize:msize;
   }
   fseek(mtl,0,SEEK_SET);
   char * buf = (char*)malloc((osize+1)*sizeof(char));
   char * str = (char*)malloc((osize+1)*sizeof(char));
   char * str1 = (char*)malloc((osize+1)*sizeof(char));
   char * str2 = (char*)malloc((osize+1)*sizeof(char));
   char * str3 = (char*)malloc((osize+1)*sizeof(char));
   char * str4 = (char*)malloc((osize+1)*sizeof(char));
   map<string,MTL> mtls;
   XML xml;
   mtls["default"]=MTL();
   MTL * cur=&mtls["default"];
   while (fgets(buf,osize,mtl)) {
      if (buf[0]=='#'||buf[0]==0)
         continue;
      if (1==sscanf(buf,"newmtl %s\n",str)) {
         mtls[str]=MTL();
         cur=&mtls[str];
		 cur->textures.push_back(textureholder());
		 cur->textures.back().index=0;
         cur->textures.back().type=TEXTURE;
	     cur->textures.back().name.push_back('w');
	     cur->textures.back().name.push_back('h');
	     cur->textures.back().name.push_back('i');
	     cur->textures.back().name.push_back('t');
	     cur->textures.back().name.push_back('e');
	     cur->textures.back().name.push_back('.');
	     cur->textures.back().name.push_back('b');
	     cur->textures.back().name.push_back('m');
	     cur->textures.back().name.push_back('p');
         continue;
      }
      wordtoupper(buf);
      float tmpblend;
      if (1==sscanf(buf,"BLEND %f\n",&tmpblend)) {
        if (tmpblend==1) {
          cur->blend_src = ONE;
          cur->blend_dst = ONE;
        }else if (tmpblend==.5) {
          cur->blend_src = SRCALPHA;
          cur->blend_dst = INVSRCALPHA;
        }else {
          cur->blend_src=ONE;
          cur->blend_dst=ZERO;
        }
      }
      if (3==sscanf(buf,"KA %f %f %f\n",&cur->ar,&cur->ag,&cur->ab)) {
        cur->aa=1;
      }
      if (3==sscanf(buf,"KS %f %f %f\n",&cur->sr,&cur->sg,&cur->sb)) {
        cur->sa=1;
      }
      if (3==sscanf(buf,"KD %f %f %f\n",&cur->dr,&cur->dg,&cur->db)) {
        cur->da=1;
      }
      if (3==sscanf(buf,"KE %f %f %f\n",&cur->er,&cur->eg,&cur->eb)) {
        cur->ea=1;
      }
      if (1==sscanf(buf,"NORMALS %f\n",&tmpblend)) {
        if (tmpblend!=0)
          cur->usenormals=true;
      
        else 
          cur->usenormals=false;
      }
      if (1==sscanf(buf,"MAP_REFLECTION %f\n",&tmpblend)) {
        if (tmpblend!=0) {
          cur->reflect=1;
        }
        else {
          cur->reflect=0;
        }
      }
      sscanf(buf,"NS %f\n",&cur->power);
      float floate,floatf,floatg;
      if (3==sscanf(buf,"detail_plane %f %f %f\n",&floate,&floatf,&floatg)) {
         cur->detailplanei.push_back(floate);
         cur->detailplanej.push_back(floatf);
         cur->detailplanek.push_back(floatg);
      }
      if (1==sscanf(buf,"illum %f\n",&floate)) {
         cur->er=floate;
         cur->eg=floate;
         cur->eb=floate;
      }
      if (1==sscanf(buf,"MAP_KD %s\n",str)) {
         if (cur->textures.empty())
           cur->textures.push_back(textureholder(0));
         cur->textures[0]=makeTextureHolder(str,0);
      }
      if (1==sscanf(buf,"MAP_KS %s\n",str)) {
         while (cur->textures.size()<=1)
           cur->textures.push_back(textureholder(cur->textures.size()));         
         cur->textures[1]=makeTextureHolder(str,1);
      }
      if (1==sscanf(buf,"MAP_KA %s\n",str)) {
         while (cur->textures.size()<=2)
           cur->textures.push_back(textureholder(cur->textures.size()));
         cur->textures[2]=makeTextureHolder(str,2);
      }
      if (1==sscanf(buf,"MAP_KE %s\n",str)) {
         while (cur->textures.size()<=3)
           cur->textures.push_back(textureholder(cur->textures.size()));         
         cur->textures[3]=makeTextureHolder(str,3);
      }
      if (1==sscanf(buf,"MAP_DETAIL %s\n",str)) {
        cur->detail=makeTextureHolder(str,0);
      }      
   }
   bool changemat=false;

   vector <VTX> vtxlist;
   vector <TEX> txclist;
   map    <TEX,int> txcmap;
   vector <int> txcmap_ii;
   map    <NORMAL,int> normalmap;
   vector <int> normalmap_ii;
   vector <NORMAL> normallist;
   map    <string,vector <FACE> > facelistlist;
   vector <FACE> facelist;
   map    <string,string> mattexmap;
   string curmat;

   vector <pair<float,float> > tex;
   while (fgets(buf,osize,obj)) {
      if (buf[0]=='#'||buf[0]=='g')
         continue;
      if (1==sscanf(buf,"usemtl %s\n",str)) {
         if (changemat) {
           // append to facelistlist
           facelistlist[curmat].insert(facelistlist[curmat].end(),facelist.begin(),facelist.end());
           facelist.clear();
         }
         curmat=str;
         changemat=true;
         continue;
      }

      if (1==sscanf(buf,"usemat %s\n",str)) {
          mtls[curmat].textures.clear();
          mtls[curmat].textures.push_back(makeTextureHolder(str,0));
      }

      GFXVertex v;

      memset(&v,0,sizeof(GFXVertex));
      if (3==sscanf(buf,"v %f %f %f\n",&v.x,&v.y,&v.z)) {
        //xml.vertices.push_back(v);
        //xml.num_vertex_references.push_back(0);
        vtxlist.push_back(VTX(v.x,v.y,v.z));
		continue;
      }
      if (3==sscanf(buf,"vn %f %f %f\n",&v.i,&v.j,&v.k)) {
        //Sharing is loussy in .obj files... so lets merge a little
        NORMAL n(-v.i,-v.j,-v.k);
        map<NORMAL,int>::iterator mi = normalmap.find(n);
        if (mi==normalmap.end()) {
            normalmap_ii.push_back(normallist.size());
            normalmap.insert(pair<NORMAL,int>(n,normallist.size()));
            normallist.push_back(n);
        } else {
            normalmap_ii.push_back((*mi).second);
        }
		continue;
      }
      if (2==sscanf(buf,"vt %f %f\n",&v.s,&v.t)) {
        //tex.push_back(pair<float,float>(v.s,v.t)); 
        //Sharing is loussy in .obj files... so lets merge a little
        TEX t(v.s,1.0f-v.t);
        map<TEX,int>::iterator mi = txcmap.find(t);
        if (mi==txcmap.end()) {
            txcmap_ii.push_back(txclist.size());
            txcmap.insert(pair<TEX,int>(t,txclist.size()));
            txclist.push_back(t);
        } else {
            txcmap_ii.push_back((*mi).second);
        }
		continue;
      }
      int rv=sscanf(buf,"f %s %s %s %s %s\n",str,str1,str2,str3,str4);
      if (rv>=2) {
          switch (rv) {
          case 5: 
              //fan - decomposed into tris. Sorry, but after fanification-stripification is done, everything will be allright again.
              {
                  vector<string> splitwhite = splitWhiteSpace(buf+1);        
                  for (int i=2; i<splitwhite.size(); i++)
                      facelist.push_back(map_face(FACE(parsePoly(splitwhite[0]),parsePoly(splitwhite[i-1]),parsePoly(splitwhite[i-2])),txcmap_ii,normalmap_ii)); 
              }
              break;
          case 4: facelist.push_back(map_face(FACE(parsePoly(str),parsePoly(str1),parsePoly(str2),parsePoly(str3)),txcmap_ii,normalmap_ii)); break;
          case 3: facelist.push_back(map_face(FACE(parsePoly(str),parsePoly(str1),parsePoly(str2)),txcmap_ii,normalmap_ii)); break;
          case 2: facelist.push_back(map_face(FACE(parsePoly(str),parsePoly(str1)),txcmap_ii,normalmap_ii)); break;
          }
          continue;
      }
   }

   // append to facelistlist
   facelistlist[curmat].insert(facelistlist[curmat].end(),facelist.begin(),facelist.end());
   facelist.clear();

   int textnum=0;
   for (map<string,vector<FACE> >::iterator it=facelistlist.begin(); it!=facelistlist.end(); it++) {
     string mat=(*it).first;

     xml.vertices.clear();
     xml.tris.clear();
     xml.quads.clear();
     xml.lines.clear();
     xml.linestrips.clear();
     xml.tristrips.clear();
     xml.quadstrips.clear();
     xml.trifans.clear();

     xml.material=mtls[mat];
     xml.textures=mtls[mat].textures;
     xml.detailtexture=mtls[mat].detail;
     for (int jjjj=0;jjjj<mtls[mat].detailplanei.size();++jjjj) {
         Mesh_vec3f v;
         v.x=mtls[mat].detailplanei[jjjj];
         v.y=mtls[mat].detailplanej[jjjj];
         v.z=mtls[mat].detailplanek[jjjj];
         xml.detailplanes.push_back(Mesh_vec3f(v));
     }
     xml.blend_src = mtls[mat].blend_src;
     xml.blend_dst = mtls[mat].blend_dst;
     xml.reflect=mtls[mat].reflect;
     if(forcenormals) 
         mtls[mat].usenormals=true;
     xml.usenormals=mtls[mat].usenormals;

     ObjToXML(xml,vtxlist,txclist,normallist,(*it).second);

     printf("%d_0: %d faces, %d vertices, %d lines, %d tris, %d quads\n",textnum,(*it).second.size(),xml.vertices.size(),xml.lines.size()/2,xml.tris.size()/3,xml.quads.size()/4);

     xmeshToBFXM(xml,outputFile,textnum==0?'c':'a',forcenormals);
     textnum++;
   }
}
