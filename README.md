nginx-responsiveindex
=====================

[![Build Status](https://travis-ci.org/duganchen/ngx-responsiveindex.svg?branch=master)](https://travis-ci.org/duganchen/ngx-responsiveindex)

Responsive boostrap-formatted indexes module for the Nginx web server.

[sample](http://htmlpreview.github.io/?https://github.com/duganchen/ngx-responsiveindex/blob/master/sample.html)

This is a drop-in replacement for [autoindex](http://nginx.org/en/docs/http/ngx_http_autoindex_module.html).
Simply replace autoindex" in nginx.conf with "responsiveindex".

The following options correspond to those for autoindex:

* responsiveindex_localtime
* responsiveindex_exact_size

There are two more options:

* *responsiveindex_bootstrap_href* can be passed the URL to load the Twitter Bootstrap CSS (it's CDN-loaded by default)
* *responsiveindex_lang* can be passed the value to set the HTML "lang" attribute to ("en" by default).
