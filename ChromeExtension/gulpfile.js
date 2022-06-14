/*
  Gulp build file for BeIDConnect Chrome Extension.

  To install Node modules: run "npm install"
  To build in debug (not domain filtering): run "gulp"
  To build in release (Jenkins): run "gulp release"

  You must have npm installed, which is part of NodeJS (http://nodejs.org/),
  and gulp ("sudo npm install --global gulp").
 */

var gulp = require('gulp'),
  del = require('del'),
  uglify = require('gulp-uglify'),
  jshint = require('gulp-jshint'),
  zip = require('gulp-zip'),
  replaceTask = require('gulp-replace-Task'),
  replace = require('gulp-replace'),
  jeditor = require('gulp-json-editor'),
  argv = require("yargs").string('extversion').argv;

var version = "2.5.0";
var ChromeVersion = "0.0.7";
var ChromeID = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA0VKd3Y3RnWfFWi8YuiV5rR6qj103pvMMvYnLuV3XoChZWCSOFvDhzRZ3XJkwdYHpN3U3wLfadxUnYkWaH+w1jRZU9opM9WZxJFwor89ajCVK9wJ21cnfVnwWwIZESaEKt7ic2v+96miDzZryHQtVFzqyGr3aF/6SXr9u2iRJuzHlyCnVEuc8NiguYmUKnL5RZuy+z+9sCK+Q1P7bfj4tIIwUGVzC2MpjaSU1NcQCO+Rk23wIcWVmzX5n3EPOx0D8vHHnSTZxA6f9JigqthsHAR3v6c4bHsjpI6GQSX5PtD4Vfy1T7iYMrxi+mZFKd+qSLkWSqssHUnQGxnhEQJvFsQIDAQAB"
var EdgeVersion = "1.0.0";
var EdgeID = "kbfoeejdnkbgifongkedipkifldkkphn"
var FirefoxVersion = "0.0.11";

// Delete the target directory
function taskclean(cb) {
  del(['target']);
  cb();
};

// jshint
function tasklint(cb) {
  return gulp.src(['./src/main/*.js'])
    .pipe(jshint())
    .pipe(jshint.reporter());
}
function tasklintV3(cb) {
  return gulp.src(['./src_v3/main/*.js'])
    .pipe(jshint())
    .pipe(jshint.reporter());
}

// Minify
function taskMinifyjs(cb) {
  return gulp.src('./src/main/*.js')
    .pipe(uglify())
    .pipe(gulp.dest('./target/chrome'))
    .pipe(gulp.dest('./target/firefox'))
};
function taskMinifyjsV3(cb) {
  return gulp.src('./src_v3/main/*.js')
    .pipe(uglify())
    .pipe(gulp.dest('./target/chrome_v3'))
    .pipe(gulp.dest('./target/firefox_v3'))
    .pipe(gulp.dest('./target/edge_v3'));
};

function taskcopypng(cb) {
  return gulp.src(['./src/main/*.png'])
    .pipe(gulp.dest('./target/chrome'))
    .pipe(gulp.dest('./target/firefox'));
};

function taskcopypngV3(cb) {
  return gulp.src(['./src_v3/main/*.png'])
    .pipe(gulp.dest('./target/chrome_v3'))
    .pipe(gulp.dest('./target/firefox_v3'))
    .pipe(gulp.dest('./target/edge_v3'));
};

function taskcopyall(cb) {
  return gulp.src(['./src/main/*.js', './src/main/*.png', './src/main/*.html'])
    .pipe(gulp.dest('./target/chrome'))
    .pipe(gulp.dest('./target/firefox'));
};
function taskcopyallV3(cb) {
  return gulp.src(['./src_v3/main/*.js', './src_v3/main/*.png', './src_v3/main/*.html'])
    .pipe(gulp.dest('./target/chrome_v3'))
    .pipe(gulp.dest('./target/firefox_v3'));
};
function taskcopyallV3Edge1(cb) {
  return gulp.src(['./src_v3/main/*.png', './src_v3/main/*.html'])
    .pipe(gulp.dest('./target/edge_v3'));
};
function taskcopyallV3Edge2(cb) {
  return gulp.src(['./src_v3/main/content.js',])
    .pipe(replace('beidconnect.page', 'beidconnectedge.page'))
    .pipe(gulp.dest('./target/edge_v3'));
};
function taskcopyallV3Edge3(cb) {
  return gulp.src('./src_v3/main/background.js')
    .pipe(replace('beidconnect.page', 'beidconnectedge.page'))
    .pipe(replace('beidconnect.background', 'beidconnectedge.background'))
    .pipe(gulp.dest('./target/edge_v3'));
};
function taskcopyallV3Edge4(cb) {
  return gulp.src('./src_v3/main/page.js')
    .pipe(replace('beidconnect.page', 'beidconnectedge.page'))
    .pipe(replace('beidconnect.background', 'beidconnectedge.background'))
    .pipe(gulp.dest('./target/edge_v3'));
};

function taskcopylocales(cb) {
  return gulp.src(['./src_v3/main/_locales/**/*'])
    .pipe(gulp.dest('./target/chrome_v3/_locales'))
    .pipe(gulp.dest('./target/firefox_v3/_locales'))
    .pipe(gulp.dest('./target/edge_v3/_locales'));
};

function taskmanifestdevchrome(cb) {
  return gulp.src('./src/main/manifest.json')
    .pipe(replaceTask({
      patterns: [
        {
          match: 'VERSION',
          replacement: ChromeVersion
        }
      ]
    }))
    .pipe(gulp.dest('./target/chrome'));
};
function taskmanifestdevchromeV3(cb) {
  return gulp.src('./src_v3/main/manifest.json')
    .pipe(replaceTask({
      patterns: [
        {
          match: 'VERSION',
          replacement: ChromeVersion
        }
      ]
    }))
    .pipe(gulp.dest('./target/chrome_v3'));
};
function taskmanifestdevfirefox(cb) {
  return gulp.src('./src/main/manifest.json')
    .pipe(jeditor(function (manifest) {
      manifest.version = FirefoxVersion;
      delete manifest.key;
      delete manifest.minimum_chrome_version;
      delete manifest.background.persistent;
      manifest.applications = { 'gecko': { 'id': 'beidconnect@bosa.be', 'strict_min_version': '57.0' } };
      return manifest;
    }))
    .pipe(gulp.dest('./target/firefox'));
};
// function taskmanifestdevfirefoxV3(cb) {
//   return gulp.src('./src_v3/main/manifest.json')
//     .pipe(jeditor(function (manifest) {
//       manifest.version = FirefoxVersion;
//       delete manifest.key;
//       delete manifest.minimum_chrome_version;
//       delete manifest.background.persistent;
//       manifest.applications = { 'gecko': { 'id': 'beidconnect@bosa.be', 'strict_min_version': '57.0' } };
//       return manifest;
//     }))
//     .pipe(gulp.dest('./target/firefox_v3'));
// };
function taskmanifestdevedgeV3(cb) {
  return gulp.src('./src_v3/main/manifest.json')
    .pipe(jeditor(function (manifest) {
      manifest.key = EdgeID;
      manifest.version = EdgeVersion;
      return manifest;
    }))
    .pipe(gulp.dest('./target/edge_v3'));
};
function taskmanifestreleasechrome() {
  return gulp.src('./src/main/manifest.json')
    .pipe(replaceTask({
      patterns: [
        {
          match: /"matches": \[.+\]/g,
          replacement: '"matches": ["https://*.belgium.be/*","https://*.fgov.be/*"]'
        },
        {
          match: 'VERSION',
          replacement: ChromeVersion
        }
      ]
    }))
    .pipe(gulp.dest('./target/chrome'));
};
function taskmanifestreleasechromeV3() {
  return gulp.src('./src_v3/main/manifest.json')
    .pipe(replaceTask({
      patterns: [
        {
          match: /"matches": \[.+\]/g,
          replacement: '"matches": ["https://*.belgium.be/*","https://*.fgov.be/*"]'
        },
        {
          match: 'VERSION',
          replacement: ChromeVersion
        }
      ]
    }))
    .pipe(gulp.dest('./target/chrome_v3'));
};
function taskmanifestreleasefirefox() {
  return gulp.src('./src/main/manifest.json')
    .pipe(jeditor(function (manifest) {
      manifest.version = FirefoxVersion;
      manifest.content_scripts[0].matches = ['https://*.belgium.be/*', 'https://*.fgov.be/*'];
      delete manifest.key;
      delete manifest.minimum_chrome_version;
      delete manifest.background.persistent;
      manifest.applications = { 'gecko': { 'id': 'beidconnect@bosa.be', 'strict_min_version': '57.0', "update_url": "https://eid.static.bosa.fgov.be/ffupdate-manifest.json" } };
      return manifest;
    }))
    .pipe(gulp.dest('./target/firefox'));
};
// function taskmanifestreleasefirefoxV3() {
//   return gulp.src('./src_v3/main/manifest.json')
//     .pipe(jeditor(function (manifest) {
//       manifest.version = FirefoxVersion;
//       manifest.content_scripts[0].matches = ['https://*.belgium.be/*', 'https://*.fgov.be/*'];
//       delete manifest.key;
//       delete manifest.minimum_chrome_version;
//       delete manifest.background.persistent;
//       manifest.applications = { 'gecko': { 'id': 'beidconnect@bosa.be', 'strict_min_version': '57.0', "update_url": "https://eid.static.bosa.fgov.be/ffupdate-manifest.json" } };
//       return manifest;
//     }))
//     .pipe(gulp.dest('./target/firefox_v3'));
// };
function taskmanifestreleaseedgeV3() {
  return gulp.src('./src_v3/main/manifest.json')
    .pipe(jeditor(function (manifest) {
      manifest.key = EdgeID;
      manifest.version = EdgeVersion;
      manifest.content_scripts[0].matches = ['https://*.belgium.be/*', 'https://*.fgov.be/*'];
      return manifest;
    }))
    .pipe(gulp.dest('./target/edge_v3'));
};
function taskzipchrome() {
  return gulp.src('./target/chrome/*')
    .pipe(zip('beidconnect-chrome-ext-' + ChromeVersion + '.zip'))
    .pipe(gulp.dest('./target'));
};
function taskzipfirefox() {
  return gulp.src('./target/firefox/*')
    .pipe(zip('beidconnect-firefox-ext-' + FirefoxVersion + '.zip'))
    .pipe(gulp.dest('./target'));
};
function taskzipchromeV3() {
  return gulp.src('./target/chrome_v3/**')
    .pipe(zip('beidconnect-chrome-ext-v3-' + ChromeVersion + '.zip'))
    .pipe(gulp.dest('./target'));
};
// function taskzipfirefoxV3() {
//   return gulp.src('./target/firefox_v3/**')
//     .pipe(zip('beidconnect-firefox-ext-v3-' + FirefoxVersion + '.zip'))
//     .pipe(gulp.dest('./target'));
// };
function taskzipedgeV3() {
  return gulp.src('./target/edge_v3/**')
    .pipe(zip('beidconnect-Edge-ext-v3-' + EdgeVersion + '.zip'))
    .pipe(gulp.dest('./target'));
};

exports.default = gulp.series(
  taskclean,
  gulp.parallel(
    gulp.series(
      tasklint,
      taskcopyall,
      gulp.parallel(
        gulp.series(
          taskmanifestdevchrome,
          taskzipchrome),
        gulp.series(
          taskmanifestdevfirefox,
          taskzipfirefox)
      )
    ),
    gulp.series(
      tasklintV3,
      taskcopyallV3,
      taskcopyallV3Edge1,
      taskcopyallV3Edge2,
      taskcopyallV3Edge3,
      taskcopyallV3Edge4,
      taskcopylocales,
      gulp.parallel(
        gulp.series(
          taskmanifestdevchromeV3,
          taskzipchromeV3),
        // gulp.series(
        //   taskmanifestdevfirefoxV3,
        //   taskzipfirefoxV3),
        gulp.series(
          taskmanifestdevedgeV3,
          taskzipedgeV3)
      )
    ),
  )
);
exports.release = gulp.series(
  taskclean,
  gulp.parallel(
    gulp.series(
      tasklint,
      taskMinifyjs,
      taskcopypng,
      gulp.parallel(
        gulp.series(
          taskmanifestreleasechrome,
          taskzipchrome),
        gulp.series(
          taskmanifestreleasefirefox,
          taskzipfirefox)
      )
    ),
    gulp.series(
      tasklintV3,
      taskMinifyjsV3,
      taskcopypngV3,
      taskcopylocales,
      gulp.parallel(
        gulp.series(
          taskmanifestreleasechromeV3,
          taskzipchromeV3),
        // gulp.series(
        //   taskmanifestreleasefirefoxV3,
        //   taskzipfirefoxV3),
        gulp.series(
          taskmanifestreleaseedgeV3,
          taskzipedgeV3)
      )
    )
  )
);
