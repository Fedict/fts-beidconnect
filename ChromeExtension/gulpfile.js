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

var ChromeVersion = "0.0.8";
var ChromeID = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA0VKd3Y3RnWfFWi8YuiV5rR6qj103pvMMvYnLuV3XoChZWCSOFvDhzRZ3XJkwdYHpN3U3wLfadxUnYkWaH+w1jRZU9opM9WZxJFwor89ajCVK9wJ21cnfVnwWwIZESaEKt7ic2v+96miDzZryHQtVFzqyGr3aF/6SXr9u2iRJuzHlyCnVEuc8NiguYmUKnL5RZuy+z+9sCK+Q1P7bfj4tIIwUGVzC2MpjaSU1NcQCO+Rk23wIcWVmzX5n3EPOx0D8vHHnSTZxA6f9JigqthsHAR3v6c4bHsjpI6GQSX5PtD4Vfy1T7iYMrxi+mZFKd+qSLkWSqssHUnQGxnhEQJvFsQIDAQAB"
var EdgeVersion = "1.0.2";
var EdgeID = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAhSZ7pV9iLxtvHvSsvKNIyPP8O91rj31fDQBecPmGTAx96axL32sHsroPiZl0fOtsbPdIAwt5CLDsxfeCZH1oxCkEO6LRe1YoQSs2X0llqS9EwZgoQBQ3k6S37qnFyBk4eJjNGw3uJgC4syuM70m85219KvbwFT8tj17f4RFIvygrWiF2nlws4+6rAIDHRImSVGS1mFutJdxir/U3dtLLrlRUusZfsLSXq8CU2Ill4FQXwBbFuRgsEELJoTsJNiu/IPPA6wI/IAdS/W+Fi1M7xutIeyqrPuNOesNDBHZxtr4JW0N7uf0LRdofeeDIXr02U1AaJ7NIdejYfBn5Mc1jtwIDAQAB"
var FirefoxVersion = "0.0.12";

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
    //.pipe(uglify())
    .pipe(gulp.dest('./target/firefox'))
};
function taskMinifyjsV3(cb) {
  return gulp.src('./src_v3/main/*.js')
    .pipe(uglify())
    .pipe(gulp.dest('./target/chrome_v3'))
    .pipe(gulp.dest('./target/edge_v3'));
};

function taskcopypng(cb) {
  return gulp.src(['./src/main/*.png'])
    .pipe(gulp.dest('./target/firefox'));
};

function taskcopypngV3(cb) {
  return gulp.src(['./src_v3/main/*.png'])
    .pipe(gulp.dest('./target/chrome_v3'))
    .pipe(gulp.dest('./target/edge_v3'));
};

function taskcopyall(cb) {
  return gulp.src(['./src/main/*.js', './src/main/*.png', './src/main/*.html'])
    .pipe(gulp.dest('./target/firefox'));
};
function taskcopyallV3(cb) {
  return gulp.src(['./src_v3/main/*.js', './src_v3/main/*.png', './src_v3/main/*.html'])
    .pipe(gulp.dest('./target/chrome_v3'))
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
    .pipe(gulp.dest('./target/edge_v3/_locales'));
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
function taskmanifestdevedgeV3(cb) {
  return gulp.src('./src_v3/main/manifest.json')
    .pipe(jeditor(function (manifest) {
      // manifest.key = EdgeID;
      delete manifest.key;
      manifest.version = EdgeVersion;
      return manifest;
    }))
    .pipe(gulp.dest('./target/edge_v3'));
};
function taskmanifestreleasechromeV3() {
  return gulp.src('./src_v3/main/manifest.json')
    .pipe(replaceTask({
      patterns: [
        {
          match: /"matches": \[.+\]/g,
          replacement: '"matches": ["https://*.belgium.be/*","https://*.fgov.be/*","https://gcloudbelgium.sharepoint.com/*"]'
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
      manifest.content_scripts[0].matches = ['https://*.belgium.be/*', 'https://*.fgov.be/*', "https://gcloudbelgium.sharepoint.com/*"];
      delete manifest.key;
      delete manifest.minimum_chrome_version;
      delete manifest.background.persistent;
      manifest.applications = { 'gecko': { 'id': 'beidconnect@bosa.be', 'strict_min_version': '57.0', "update_url": "https://eid.static.bosa.fgov.be/ffupdate-manifest.json" } };
      return manifest;
    }))
    .pipe(gulp.dest('./target/firefox'));
};
function taskmanifestreleaseedgeV3() {
  return gulp.src('./src_v3/main/manifest.json')
    .pipe(jeditor(function (manifest) {
      // manifest.key = EdgeID;
      delete manifest.key;
      manifest.version = EdgeVersion;
      manifest.content_scripts[0].matches = ['https://*.belgium.be/*', 'https://*.fgov.be/*', "https://gcloudbelgium.sharepoint.com/*"];
      return manifest;
    }))
    .pipe(gulp.dest('./target/edge_v3'));
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
        // gulp.series(
        //   taskmanifestdevchrome,
        //   taskzipchrome),
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
        // gulp.series(
        //   taskmanifestreleasechrome,
        //   taskzipchrome),
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
        gulp.series(
          taskmanifestreleaseedgeV3,
          taskzipedgeV3)
      )
    )
  )
);
