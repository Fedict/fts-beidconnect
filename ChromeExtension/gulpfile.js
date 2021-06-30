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
    sequence = require('run-sequence'),
    replace = require('gulp-replace-task'),
    jeditor = require('gulp-json-editor'),
    argv = require("yargs").string('extversion').argv;

var version = (argv.extversion !== undefined ? argv.extversion : '0');

gulp.task('default', function() {
    sequence('clean', 'lint', 'copy-all', 'manifest-dev-chrome', 'manifest-dev-firefox', 'zip-chrome', 'zip-firefox');
});

gulp.task('release', function() {
   sequence('clean', 'lint', 'minify-js', 'copy-png', 'manifest-release-chrome', 'manifest-release-firefox', 'zip-chrome', 'zip-firefox');
});

gulp.task('clean', function () {
    del(['target']);
});

gulp.task('lint', function () {
    return gulp.src(['./src/main/*.js'])
      .pipe(jshint())
      .pipe(jshint.reporter());
});

gulp.task('minify-js', function () {
    return gulp.src('./src/main/*.js')
      .pipe(uglify())
      .pipe(gulp.dest('./target/chrome'))
      .pipe(gulp.dest('./target/firefox'));
});

gulp.task('copy-png', function () {
    return gulp.src(['./src/main/*.png'])
      .pipe(gulp.dest('./target/chrome'))
      .pipe(gulp.dest('./target/firefox'));
});

gulp.task('copy-all', function () {
    return gulp.src(['./src/main/*.js','./src/main/*.png', './src/main/*.html'])
      .pipe(gulp.dest('./target/chrome'))
      .pipe(gulp.dest('./target/firefox'));
});

gulp.task('manifest-dev-chrome', function () {
    return gulp.src('./src/main/manifest.json')
    .pipe(replace({
      patterns: [
        {match: 'VERSION',
         replacement: version}
      ]
    }))
    .pipe(gulp.dest('./target/chrome'));
});

gulp.task('manifest-dev-firefox', function() {
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
});

gulp.task('manifest-release-chrome', function () {
    return gulp.src('./src/main/manifest.json')
    .pipe(replace({
      patterns: [
        {match: /"matches": \[.+\]/g,
                 replacement: '"matches": ["https://*.belgium.be/*"]'},
        {match: 'VERSION',
         replacement: version}
      ]
    }))
    .pipe(gulp.dest('./target/chrome'));
});

gulp.task('manifest-release-firefox', function() {
    return gulp.src('./src/main/manifest.json')
    .pipe(jeditor(function(manifest) {
	manifest.version = version;
	manifest.content_scripts[0].matches = ['https://*.belgium.be/*'];
	delete manifest.key;
	delete manifest.minimum_chrome_version;
	delete manifest.background.persistent;
	manifest.applications = { 'gecko': { 'id':'beidconnect@bosa.be','strict_min_version':'57.0', "update_url": "https://eid.static.bosa.fgov.be/ffupdate-manifest.json" }};
	return manifest;
    }))
    .pipe(gulp.dest('./target/firefox'));
});

gulp.task('zip-chrome', function () {
    return gulp.src('./target/chrome/*')
      .pipe(zip('beidconnect-chrome-ext-' + version + '.zip'))
      .pipe(gulp.dest('./target'));
});

gulp.task('zip-firefox', function () {
    return gulp.src('./target/firefox/*')
      .pipe(zip('beidconnect-firefox-ext-' + version + '.zip'))
      .pipe(gulp.dest('./target'));
});
