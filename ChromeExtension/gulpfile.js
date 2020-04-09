/*
  Gulp build file for eIDLink Chrome Extension.

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
    argv = require("yargs").string('extversion').argv;

var version = (argv.extversion !== undefined ? argv.extversion : '0');

gulp.task('default', function() {
    sequence('clean', 'lint', 'copy-all', 'manifest-dev', 'zip');
});

gulp.task('release', function() {
   sequence('clean', 'lint', ['minify-js', 'copy-png', 'manifest-release'], 'zip');
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
      .pipe(gulp.dest('./target/ext'));
});

gulp.task('copy-png', function () {
    return gulp.src(['./src/main/*.png'])
      .pipe(gulp.dest('./target/ext'))
});

gulp.task('copy-all', function () {
    return gulp.src(['./src/main/*.js','./src/main/*.png', './src/main/*.html'])
      .pipe(gulp.dest('./target/ext'))
});

gulp.task('manifest-dev', function () {
	return gulp.src('./src/main/manifest.json')
    .pipe(replace({
      patterns: [
        {match: 'VERSION',
         replacement: version}
      ]
    }))
    .pipe(gulp.dest('./target/ext'));
});

gulp.task('manifest-release', function () {
	return gulp.src('./src/main/manifest.json')
    .pipe(replace({
      patterns: [
        {match: /"matches": \[.+\]/g,
                 replacement: '"matches": ["https://*.belgium.be/*", "https://*.zetes.be/*"]'},
        {match: 'VERSION',
         replacement: version}
      ]
    }))
    .pipe(gulp.dest('./target/ext'));
});

gulp.task('zip', function () {
    return gulp.src('./target/ext/*')
      .pipe(zip('eidlink-chrome-ext-' + version + '.zip'))
      .pipe(gulp.dest('./target'));
});
