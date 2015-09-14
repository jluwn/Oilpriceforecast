#include <windows.h>
#include <wininet.h>
#include <io.h>
#include <cstdio>
#include <conio.h>
#include <queue>
#include <ctime>
#include <map>
#include <set>
#include <string>
#include <iostream>
using namespace std;
#pragma comment( lib, "wininet.lib" )

namespace PV {
  const int buffsize = 1000000;
  char buff[buffsize];
}

namespace tool {
  set <char> invisible;
  void init() {
    const char *inv = "\n\r \t";
    for (int i = 0; inv[i]; i++) {
      invisible.insert(inv[i]);
    }
  }
  bool isNum(const char &x) {
    return x <= '9' && x >= '0';
  }
  bool isNum(const string &x) {
    if (x.empty()) return 0;
    for (int i = 0; i < x.size(); i++) {
      if (!isNum(x[i])) return 0;
    }
    return 1;
  }
  void trim(string &x) {
    int st = 0;
    int en = x.size() - 1;
    while (invisible.count(x[st]) && st <= en) st++;
    while (invisible.count(x[en]) && st <= en) en--;
    x = string(x.begin() + st, x.begin() + en - 1);
  }
  vector<string> split(const string &x, char sp) {
    vector<string> res(1);
    for (int i = 0; i < (int)x.size(); i++) {
      if (x[i] == sp) {
        if (!res.back().empty()) res.push_back(string());
      }
      else {
        res.back().push_back(x[i]);
      }
    }
    if (res.back().empty()) res.pop_back();
    return res;
  }
  bool fix(string &a, const string &b) {
    for (int i = 0; i < b.size(); i++) {
      if (a[i] != b[i]) return 0;
    }
    a = string(a.begin() + b.size(), a.end());
    return 1;
  }
  class trie {
  public:
    static const int sigma = 256;
    vector < vector < int > > d;
    trie() : d(1, vector < int >(sigma, 0)) {}
    void add(const char *x) {
      int idx = 0;
      for (int i = 0; !i || x[i - 1]; i++) {
        int &cur = d[idx][x[i]];
        if (!cur) {
          cur = d.size();
          d.push_back(vector < int >(sigma, 0));
        }
        idx = cur;
      }
    }
    bool find(const char *x) {
      int idx = 0;
      for (int i = 0; !i || x[i - 1]; i++) {
        int &cur = d[idx][x[i]];
        if (!cur) return 0;
        else idx = cur;
      }
      return 1;
    }
    void clear() {
      d = vector < vector < int > >(1, vector < int >(sigma, 0));
    }
  };
}

namespace System {
  const char *BufferPath = "E:/Desktop/BBCdata";
  bool fileExists(string x) {
    return _access((BufferPath + x).c_str(), 0) != -1;
  }
  bool addFolder(string x) {
    return CreateDirectory((BufferPath + x).c_str(), 0);
  }
  class file {
  public:
    FILE *p;
    file(string path, const char *opt) {
      vector<string> tmp = tool::split(path, '/');
      string cur;
      for (int i = 0; i < tmp.size() - 1; i++) {
        cur += "/" + tmp[i];
        if (!fileExists(cur) && !addFolder(cur)) {
          p = 0;
          return;
        }
      }
      p = fopen((BufferPath + path).c_str(), opt);
    }
    ~file() {
      if (valid()) fclose(p);
    }
    bool valid() {
      return p;
    }
    string getl() {
      using namespace PV;
      if (!fgets(buff, sizeof(buff), p)) return "<END>";
      for (int i = 0; buff[i] && buff[i + 1]; i++) {
        if (buff[i] == '=' && buff[i + 1] == '=') buff[i] = 0;///去注释
      }
      if (tool::invisible.count(buff[strlen(buff) - 1])) buff[strlen(buff) - 1] = 0;
      return buff;
    }
    void print(const char *format, ...) {
      va_list args;
      va_start(args, format);
      vfprintf(p, format, args);
      va_end(args);
    }
    int scan(const char *format, ...) {
      va_list args;
      va_start(args, format);
      int res = vfscanf(p, format, args);
      va_end(args);
      return res;
    }
  };
  bool addFile(string path, const string &content) {
    file out(path, "w");
    if (!out.valid()) return 0;
    out.print("%s", content.c_str());
    return 1;
  }
}

namespace net{
  const char *MIndexPath = "http://www.bbc.com/news/";
  string download(string Url){
    using namespace PV;
    ULONG Number;
    HINTERNET hSession = InternetOpen((LPCSTR)"RookIE/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    bool res = 1;
    string data;
    if (hSession){
      HINTERNET handle2 = InternetOpenUrl(hSession, (LPCSTR)(MIndexPath + Url).c_str(), NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
      if (handle2){
        InternetReadFile(handle2, buff, buffsize - 1, &Number);
        if (Number > 200){
          while (Number > 0){
            for (int i = 0; i < Number; i++) {
              data.push_back(buff[i]);
            }
            InternetReadFile(handle2, buff, buffsize - 1, &Number);
          }
        }
        else
          res = 0;
        InternetCloseHandle(handle2);
      }
      InternetCloseHandle(hSession);
    }
    return res ? data : "<ERROR>";
  }
}

namespace editor {
  set<string> used;
  set<string> index;
  deque<string> waiting;

  bool loadIndex() {
    index.clear();
    System::file in("/config/index.dat", "r");
    if (!in.valid()) return 0;
    using namespace PV;
    index.insert("");
    while (in.scan("%s", buff) == 1) {
      index.insert(buff);
    }
    return 1;
  }
  bool loadUsed() {
    used.clear();
    System::file in("/config/used.dat", "r");
    if (!in.valid()) return 0;
    using namespace PV;
    while (in.scan("%s", buff) == 1) {
      used.insert(buff);
    }
    return 1;
  }
  bool loadWaiting() {
    waiting.clear();
    System::file in("/config/waiting.dat", "r");
    if (!in.valid()) return 0;
    using namespace PV;
    while (in.scan("%s", buff) == 1) {
      waiting.push_back(buff);
    }
    return 1;
  }
  bool init() {
    return loadIndex() && loadUsed() && loadWaiting();
  }

  class Page {
    //底层，需保证正确
  public:
    vector<string> data;
    string content;
    Page(const string &x) {//传入网址的最后部分
      vector<string> tmp = tool::split(x, '-');
      //建立路径
      string path = "/pages";
      int cp = 0;
      while (cp < tmp.size()) {
        string cur = "/" + tmp[cp++];
        while (cp < tmp.size() && !System::fileExists(path + cur)) {
          cur += '-' + tmp[cp++];
        }
        path += cur;
        data.push_back(cur);
      }
    }
    int getType() {
      return !data.empty() && tool::isNum(data.back());
    }
    string toUrl() {//相对网址（最后半部分）
      string res;
      for (int i = 0; i < data.size(); i++) {
        if (i) res += '-';
        res += data[i];
      }
      return res;
    }
    bool isDone() {
      return used.count(toUrl());
    }
    bool run() {
      if (getType() == 1 && isDone()) {
        return 0;
      }
      content = net::download(toUrl());

      //测试数据
      /*System::file debug("/zhuye.txt", "r");
      fscanf(debug.p, "%[^\0]", PV::buff);
      content = PV::buff;*/

      if (content == "<ERROR>") return 0;
      analyse();
      if (getType() == 1) return getNews();
      else return 1;
    }
    void analyse() {
      vector<string> res;
      string cur;
      int inq = 0;
      for (int i = 0; i < content.size(); i++) {
        if (content[i] == '\"') {
          if (inq) {
            if (!cur.empty()) {
              res.push_back(cur);
            }
          }
          else {
            cur.clear();
          }
          inq = !inq;
        }
        else if (tool::invisible.count(content[i])) {
          inq = 0;
        }
        else {
          if (inq) cur.push_back(content[i]);
        }
      }
      for (int i = 0; i < res.size(); i++) {
        normal(res[i]);
        if (res[i] == "<ERROR>") continue;
        if (isIndex(res[i])) {//主页属性
          if (!index.count(res[i]))
            addIndex(res[i]);
        }
        else {//新闻页属性
          if (!used.count(res[i])) {
            addNews(res[i]);
          }
        }
      }
    }
    void normal(string &x) {
      using namespace tool;
      if (x[x.size() - 1] == '/') x.resize(x.size() - 1);
      fix(x, "http://");
      fix(x, "www.bbc.com");
      if (!fix(x, "/news/") || x.size() > 300) x = "<ERROR>";
    }
    bool isIndex(string &x) {
      return x.empty() || !tool::isNum(x[x.size() - 1]);
    }
    bool addIndex(string &x) {
      index.insert(x);
      System::file out("/config/index.dat", "a");
      if (!out.valid()) return 0;
      out.print("%s\n", x.c_str());
      return 1;
    }
    bool addNews(string &x) {
      waiting.push_back(x);
      System::file out("/config/waiting.dat", "a");
      if (!out.valid()) return 0;
      out.print("%s\n", x.c_str());
      return 1;
    }

  private:
    bool readNews() {//未完成
      int y, m, d;
      getTime(y, m, d);
      string ttl = getTitle();
      string news = getNews();
      if (ttl == "<ERROR>" || news == "<ERROR>") return 0;
      string path = "/info/";
      System::file out()
      used.insert(toUrl());
      return 1;
    }
  };
}

char a[100000];

int main(){
  tool::init();
  editor::init();
  editor::Page x("");
  x.run();
  system("pause");
}
