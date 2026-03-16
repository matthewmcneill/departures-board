/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: webgui/pages.h
 * Description: Contains raw HTML and JavaScript stored in PROGMEM for the 
 *              manual firmware update, file upload, and success pages.
 *
 * Exported Assets:
 * - updatePage: The HTML/JS source for the /update firmware upload page.
 * - uploadPage: The HTML source for the /upload filesystem upload page.
 * - successPage: The HTML source for the successful upload confirmation page.
 */

#pragma once
#include <Arduino.h>
// Local firmware updates via /update Web GUI
static const char updatePage[] PROGMEM =
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<html><body style=\"font-family:Helvetica,Arial,sans-serif\"><h2>Departures Board Manual Update</h2><p>Upload a <b>firmware.bin</b> file.</p>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('Progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')"
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script></body></html>";

// /upload page
static const char uploadPage[] PROGMEM =
"<html><body style=\"font-family:Helvetica,Arial,sans-serif\">"
"<h2>Upload a file to the file system</h2><form method='post' enctype='multipart/form-data'><input type='file' name='name'>"
"<input class='button' type='submit' value='Upload'></form></body></html>";

// /success page
static const char successPage[] PROGMEM =
"<html><body style=\"font-family:Helvetica,Arial,sans-serif\"><h3>Upload completed successfully.</h3>\n"
"<p><a href=\"/dir\">List file system directory</a></p>\n"
"<h2>Upload another file</h2><form method=\"post\" action=\"/upload\" enctype=\"multipart/form-data\"><input type=\"file\" name=\"name\"><input class=\"button\" type=\"submit\" value=\"Upload\"></form>\n"
"</body></html>";
