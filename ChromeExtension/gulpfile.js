/*
  Gulp build file for BeIDConnect Chrome Extension.

  To install Node modules: run "npm install"
  To build: run "gulp [--extversion X]"
  To release (Jenkins): run "gulp release --extversion Y"

  You must have npm installed, which is part of NodeJS (http://nodejs.org/),
  and gulp ("sudo npm install --global gulp").
 */

var gulp = require('gulp'),
    del = require('del'),
    uglify = require('gulp-uglify'),
    jshint = require('gulp-jshint'),
    zip = require('gulp-zip'),
    replace = require('gulp-replace-task'),
    jeditor = require('gulp-json-editor'),
    argv = require("yargs").string('extversion').argv;

//var version = (argv.extversion !== undefined ? argv.extversion : '0');
var version = "2.4.0";

function taskclean(cb) {
    del(['target']);
    cb();
  };

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

function taskMinifyjs(cb) {
    return gulp.src('./src/main/*.js')
      .pipe(uglify())
      .pipe(gulp.dest('./target/chrome'))
      .pipe(gulp.dest('./target/firefox'))
      .pipe(gulp.dest('./target/edge'));
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
      .pipe(gulp.dest('./target/firefox'))
      .pipe(gulp.dest('./target/edge'));
};

function taskcopypngV3(cb) {
    return gulp.src(['./src_v3/main/*.png'])
      .pipe(gulp.dest('./target/chrome_v3'))
      .pipe(gulp.dest('./target/firefox_v3'))
      .pipe(gulp.dest('./target/edge_v3'));
};

function taskcopyall(cb) {
    return gulp.src(['./src/main/*.js','./src/main/*.png', './src/main/*.html'])
      .pipe(gulp.dest('./target/chrome'))
      .pipe(gulp.dest('./target/firefox'))
      .pipe(gulp.dest('./target/edge'));
};
function taskcopyallV3(cb) {
    return gulp.src(['./src_v3/main/*.js','./src_v3/main/*.png', './src_v3/main/*.html'])
      .pipe(gulp.dest('./target/chrome_v3'))
      .pipe(gulp.dest('./target/firefox_v3'))
      .pipe(gulp.dest('./target/edge_v3'));
};

function taskmanifestdevchrome(cb) {
    return gulp.src('./src/main/manifest.json')
    .pipe(replace({
      patterns: [
        {match: 'VERSION',
         replacement: version}
      ]
    }))
    .pipe(gulp.dest('./target/chrome'));
};
function taskmanifestdevchromeV3(cb) {
    return gulp.src('./src_v3/main/manifest.json')
    .pipe(replace({
      patterns: [
        {match: 'VERSION',
         replacement: version}
      ]
    }))
    .pipe(gulp.dest('./target/chrome_v3'));
};
function taskmanifestdevfirefox(cb) {
    return gulp.src('./src/main/manifest.json')
    .pipe(jeditor(function(manifest) {
	manifest.version = version;
	delete manifest.key;
	delete manifest.minimum_chrome_version;
	delete manifest.background.persistent;
	manifest.applications = { 'gecko': { 'id':'beidconnect@bosa.be','strict_min_version':'57.0'}};
	return manifest;
    }))
    .pipe(gulp.dest('./target/firefox'));
};
function taskmanifestdevfirefoxV3(cb) {
    return gulp.src('./src_v3/main/manifest.json')
    .pipe(jeditor(function(manifest) {
	manifest.version = version;
	delete manifest.key;
	delete manifest.minimum_chrome_version;
	delete manifest.background.persistent;
	manifest.applications = { 'gecko': { 'id':'beidconnect@bosa.be','strict_min_version':'57.0'}};
	return manifest;
    }))
    .pipe(gulp.dest('./target/firefox_v3'));
};
function taskmanifestdevedge(cb) {
    return gulp.src('./src/main/manifest.json')
    .pipe(replace({
      patterns: [
        {match: 'VERSION',
         replacement: version}
      ]
    }))
    .pipe(gulp.dest('./target/edge'));
};
function taskmanifestdevedgeV3(cb) {
    return gulp.src('./src_v3/main/manifest.json')
    .pipe(replace({
      patterns: [
        {match: 'VERSION',
         replacement: version}
      ]
    }))
    .pipe(gulp.dest('./target/edge_v3'));
};
function taskmanifestreleasechrome() {
    return gulp.src('./src/main/manifest.json')
    .pipe(replace({
      patterns: [
        {match: /"matches": \[.+\]/g,
                 replacement: '"matches": ["https://*.belgium.be/*","https://*.fgov.be/*"]'},
        {match: 'VERSION',
         replacement: version}
      ]
    }))
    .pipe(gulp.dest('./target/chrome'));
};
function taskmanifestreleasechromeV3() {
    return gulp.src('./src_v3/main/manifest.json')
    .pipe(replace({
      patterns: [
        {match: /"matches": \[.+\]/g,
                 replacement: '"matches": ["https://*.belgium.be/*","https://*.fgov.be/*"]'},
        {match: 'VERSION',
         replacement: version}
      ]
    }))
    .pipe(gulp.dest('./target/chrome_v3'));
};
function taskmanifestreleasefirefox() {
    return gulp.src('./src/main/manifest.json')
    .pipe(jeditor(function(manifest) {
      manifest.version = version;
      manifest.content_scripts[0].matches = ['https://*.belgium.be/*','https://*.fgov.be/*'];
      delete manifest.key;
      delete manifest.minimum_chrome_version;
      delete manifest.background.persistent;
      manifest.applications = { 'gecko': { 'id':'beidconnect@bosa.be','strict_min_version':'57.0', "update_url": "https://eid.static.bosa.fgov.be/ffupdate-manifest.json" }};
      return manifest;
    }))
    .pipe(gulp.dest('./target/firefox'));
};
function taskmanifestreleasefirefoxV3() {
    return gulp.src('./src_v3/main/manifest.json')
    .pipe(jeditor(function(manifest) {
      manifest.version = version;
      manifest.content_scripts[0].matches = ['https://*.belgium.be/*','https://*.fgov.be/*'];
      delete manifest.key;
      delete manifest.minimum_chrome_version;
      delete manifest.background.persistent;
      manifest.applications = { 'gecko': { 'id':'beidconnect@bosa.be','strict_min_version':'57.0', "update_url": "https://eid.static.bosa.fgov.be/ffupdate-manifest.json" }};
      return manifest;
    }))
    .pipe(gulp.dest('./target/firefox_v3'));
};
function taskmanifestreleaseedge() {
  return gulp.src('./src/main/manifest.json')
  .pipe(replace({
    patterns: [
      {match: /"matches": \[.+\]/g,
               replacement: '"matches": ["https://*.belgium.be/*","https://*.fgov.be/*"]'},
      {match: 'VERSION',
       replacement: version}
    ]
  }))
  .pipe(gulp.dest('./target/edge'));
};
function taskmanifestreleaseedgeV3() {
  return gulp.src('./src_v3/main/manifest.json')
  .pipe(replace({
    patterns: [
      {match: /"matches": \[.+\]/g,
               replacement: '"matches": ["https://*.belgium.be/*","https://*.fgov.be/*"]'},
      {match: 'VERSION',
       replacement: version}
    ]
  }))
  .pipe(gulp.dest('./target/edge_v3'));
};
function taskzipchrome() {
    return gulp.src('./target/chrome/*')
      .pipe(zip('beidconnect-chrome-ext-' + version + '.zip'))
      .pipe(gulp.dest('./target'));
};
function taskzipfirefox() {
    return gulp.src('./target/firefox/*')
      .pipe(zip('beidconnect-firefox-ext-' + version + '.zip'))
      .pipe(gulp.dest('./target'));
};
function taskzipedge() {
    return gulp.src('./target/edge/*')
      .pipe(zip('beidconnect-Edge-ext-' + version + '.zip'))
      .pipe(gulp.dest('./target'));
};
function taskzipchromeV3() {
    return gulp.src('./target/chrome_v3/*')
      .pipe(zip('beidconnect-chrome-ext-v3-' + version + '.zip'))
      .pipe(gulp.dest('./target'));
};
function taskzipfirefoxV3() {
    return gulp.src('./target/firefox_v3/*')
      .pipe(zip('beidconnect-firefox-ext-v3-' + version + '.zip'))
      .pipe(gulp.dest('./target'));
};
function taskzipedgeV3() {
    return gulp.src('./target/edge_v3/*')
      .pipe(zip('beidconnect-Edge-ext-v3-' + version + '.zip'))
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
          taskzipfirefox),
        gulp.series(
          taskmanifestdevedge,
          taskzipedge)
      )
    ),
    gulp.series(
      tasklintV3,
      taskcopyallV3,
      gulp.parallel(
        gulp.series(
          taskmanifestdevchromeV3,
          taskzipchromeV3),
        gulp.series(
          taskmanifestdevfirefoxV3,
          taskzipfirefoxV3),
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
          taskzipfirefox),
        gulp.series(
          taskmanifestreleaseedge,
          taskzipedge)
        )
    ),
    gulp.series(
      tasklintV3,
      taskMinifyjsV3,
      taskcopypngV3,
      gulp.parallel(
        gulp.series(
          taskmanifestreleasechromeV3,
          taskzipchromeV3),
        gulp.series(
          taskmanifestreleasefirefoxV3,
          taskzipfirefoxV3),
        gulp.series(
          taskmanifestreleaseedgeV3,
          taskzipedgeV3)
        )
     )
  )
  );
