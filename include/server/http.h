#pragma once

#include <server/version.h>

#include <boost/beast.hpp>

class http {
public:
  boost::beast::string_view static mime_type(boost::beast::string_view path);

  std::string static path_cat(boost::beast::string_view base, boost::beast::string_view path);

  template <class Body, class Allocator>
  boost::beast::http::response<boost::beast::http::string_body> static bad_request(
      boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req,
      boost::beast::string_view why) {
    boost::beast::http::response<boost::beast::http::string_body> res{
        boost::beast::http::status::bad_request, req.version()};
    res.set(boost::beast::http::field::server, "ZenConn " + std::string(SERVER_VERSION));
    res.set(boost::beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = why;
    res.prepare_payload();
    return res;
  }

  template <class Body, class Allocator>
  boost::beast::http::response<boost::beast::http::string_body> static not_found(
      boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req,
      boost::beast::string_view target) {
    boost::beast::http::response<boost::beast::http::string_body> res{
        boost::beast::http::status::not_found, req.version()};
    res.set(boost::beast::http::field::server, "ZenConn " + std::string(SERVER_VERSION));
    res.set(boost::beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "The resource '" + std::string(target) + "' was not found.";
    res.prepare_payload();
    return res;
  }

  template <class Body, class Allocator>
  boost::beast::http::response<boost::beast::http::string_body> static server_error(
      boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req,
      boost::beast::string_view what) {
    boost::beast::http::response<boost::beast::http::string_body> res{
        boost::beast::http::status::internal_server_error, req.version()};
    res.set(boost::beast::http::field::server, "ZenConn " + std::string(SERVER_VERSION));
    res.set(boost::beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "An error occurred: '" + std::string(what) + "'";
    res.prepare_payload();
    return res;
  }

  template <class Body, class Allocator>
  boost::beast::http::response<boost::beast::http::empty_body> static ok(
      boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req) {
    boost::beast::http::response<boost::beast::http::empty_body> res{boost::beast::http::status::ok,
                                                                     req.version()};
    res.set(boost::beast::http::field::server, "ZenConn " + std::string(SERVER_VERSION));
    res.keep_alive(req.keep_alive());
    return res;
  }

  template <class Body, class Allocator>
  boost::beast::http::response<boost::beast::http::file_body> static download(
      boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req,
      boost::beast::http::file_body::value_type& body) {
    boost::beast::http::response<boost::beast::http::file_body> res{
        std::piecewise_construct, std::make_tuple(std::move(body)),
        std::make_tuple(boost::beast::http::status::ok, req.version())};
    res.set(boost::beast::http::field::server, "ZenConn " + std::string(SERVER_VERSION));
    res.content_length(body.size());
    return res;
  }

  template <class Body, class Allocator> boost::beast::http::message_generator static handle(
      boost::beast::string_view doc_root,
      boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>&& req) {
    if (req.method() != boost::beast::http::verb::get
        && req.method() != boost::beast::http::verb::head) {
      return http::bad_request(req, "Unknown HTTP-method");
    }

    if (req.target() == "/exception") return http::server_error(req, "User-triggered error");

    if (req.target().empty() || req.target()[0] != '/'
        || req.target().find("..") != boost::beast::string_view::npos)
      return http::bad_request(req, "Illegal request-target");

    std::string path = path_cat(doc_root, req.target());
    if (req.target().back() == '/') path.append("index.html");

    boost::beast::error_code ec;
    boost::beast::http::file_body::value_type body;
    body.open(path.c_str(), boost::beast::file_mode::scan, ec);

    if (ec == boost::beast::errc::no_such_file_or_directory)
      return http::not_found(req, req.target());

    if (ec) return http::server_error(req, ec.message());

    auto const size = body.size();

    if (req.method() == boost::beast::http::verb::head) {
      boost::beast::http::response<boost::beast::http::empty_body> res = http::ok(req);
      res.set(boost::beast::http::field::content_type, mime_type(path));
      res.content_length(size);
      res.keep_alive(req.keep_alive());
      return res;
    }

    boost::beast::http::response<boost::beast::http::file_body> res = http::download(req, body);
    res.set(boost::beast::http::field::content_type, mime_type(path));
    res.keep_alive(req.keep_alive());
    return res;
  }
};