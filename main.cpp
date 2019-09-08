#include <boost/filesystem.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/qi.hpp>

#include <asyncplusplus/include/async++.h>

#include <complex>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>

using namespace boost::spirit;
using namespace boost::filesystem;

struct ModuleInfo {
  struct Head {
    std::string id;
  };
  struct Body {
    std::string path;
    std::string url;
  };
  Head head;
  Body body;
};

BOOST_FUSION_ADAPT_STRUCT(ModuleInfo::Head, (std::string, id))
BOOST_FUSION_ADAPT_STRUCT(ModuleInfo::Body, (std::string, path),
                          (std::string, url))
BOOST_FUSION_ADAPT_STRUCT(ModuleInfo, (ModuleInfo::Head, head),
                          (ModuleInfo::Body, body))

template <typename Iterator, typename Skipper>
struct ModuleParser : qi::grammar<Iterator, std::list<ModuleInfo>(), Skipper> {
  struct HeadParser : qi::grammar<Iterator, ModuleInfo::Head(), Skipper> {
    HeadParser() : HeadParser::base_type(start) {
      using ascii::char_;
      using qi::lexeme;
      using qi::lit;
      quoted_string %= lexeme['"' >> +(char_ - '"') >> '"'];
      start %= lit('[') >> "submodule" >> quoted_string >> ']';
    }
    qi::rule<Iterator, std::string(), Skipper> quoted_string;
    qi::rule<Iterator, ModuleInfo::Head(), Skipper> start;
  };
  struct BodyParser : qi::grammar<Iterator, ModuleInfo::Body(), Skipper> {
    BodyParser() : BodyParser::base_type(start) {
      path %= lit("path") >> "=" >> +(qi::char_ - qi::eol);
      url %= lit("url") >> "=" >> +(qi::char_ - qi::eol);
      start %= path >> qi::eol >> url;
    }
    qi::rule<Iterator, ModuleInfo::Body(), Skipper> start;

  private:
    qi::rule<Iterator, std::string(), Skipper> path, url;
  };
  HeadParser hp;
  BodyParser bp;
  qi::rule<Iterator, ModuleInfo(), Skipper> module;
  qi::rule<Iterator, std::list<ModuleInfo>(), Skipper> modules;
  ModuleParser()
      : ModuleParser::base_type(modules), hp(HeadParser()), bp(BodyParser()) {
    module %= hp >> qi::eol >> bp;
    modules %= module >> *(qi::eol >> module) >> -(qi::eol);
  }
};

int main() {
  ModuleParser<std::string::iterator, ascii::blank_type> parser;
  std::ifstream in("./.gitmodules");
  in.unsetf(std::ios::skipws);
  std::stringstream stream;
  std::string str;
  std::list<ModuleInfo> infos;
  in >> stream.rdbuf();
  str = stream.str();

  int i = 0;
  auto iter = str.begin();
  bool r = phrase_parse(iter, str.end(), parser, ascii::blank, infos);
  if (r) {
    std::string git_command("git clone -q ");
    async::parallel_for(infos, [&git_command](ModuleInfo &info){
				 std::cout << "Cloning " << info.head.id << " into " << info.body.path << std::endl;
				 std::string spec = git_command + info.body.url + " " + info.body.path;
				 int code = system(spec.c_str());
				 if (code == 0) {
				   std::cout << "[START] " << info.head.id << " finished." << std::endl;
				 }
				 else {
				   std::cerr << "[STOP] " << info.head.id << " failed." << std::endl;
				 }
			       });
  } else {
    std::cout << "Parsing failed\n";
  }
  return 0;
}
