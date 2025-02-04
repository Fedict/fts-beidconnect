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
    replace = require('gulp-replace'),
    jeditor = require('gulp-json-editor'),
    argv = require("yargs").string('extversion').argv,
    rename = require('gulp-rename');

var ChromeVersion = "1.0.3";
var EdgeVersion = "1.0.3";
var FirefoxVersion = "0.0.16";
// Safari version is coded inside the \SafariAppExtension\SafariAppExtension Extension\SafariExtensionHandler.mm source file

// https://developer.chrome.com/docs/extensions/develop/concepts/match-patterns
const AllowedDomains = ['https://*.belgium.be/*', 'https://*.fgov.be/*', 'https://gcloudbelgium.sharepoint.com/*'];

//#region clean

// Delete the target directory
function taskclean(cb) {
    del(['target']);
    cb();
};

//#region jshint

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

//#region Minify & copy

function taskMinifyjs(cb) {
    return gulp.src('./src/main/*.js')
        //.pipe(uglify())
        .pipe(replace('EXTENSIONPLATFORM', 'firefox'))
        .pipe(gulp.dest('./target/firefox'))
};
function taskMinifyjsV3Chrome(cb) {
    return gulp.src('./src_v3/main/*.js')
        .pipe(replace('EXTENSIONPLATFORM', 'chrome'))
        .pipe(uglify())
        .pipe(gulp.dest('./target/chrome_v3'))
};
function taskMinifyjsV3Edge(cb) {
    return gulp.src('./src_v3/main/*.js')
        .pipe(replace('EXTENSIONPLATFORM', 'edge'))
        .pipe(uglify())
        .pipe(gulp.dest('./target/edge_v3'));
};
function taskjsV3Firefox(cb) {
    return gulp.src('./src_v3/main/*.js')
        .pipe(replace('EXTENSIONPLATFORM', 'firefox'))
        .pipe(gulp.dest('./target/firefox_v3'));
};

function taskcopypng(cb) {
    return gulp.src(['./src/main/*.png'])
        .pipe(gulp.dest('./target/firefox'));
};

function taskcopypngV3(cb) {
    return gulp.src(['./src_v3/main/*.png'])
        .pipe(gulp.dest('./target/chrome_v3'))
        .pipe(gulp.dest('./target/firefox_v3'))
        .pipe(gulp.dest('./target/edge_v3'));
};

function taskcopyallV2FireFox(cb) {
    return gulp.src(['./src/main/*.js', './src/main/*.png', './src/main/*.html'])
        .pipe(replace('EXTENSIONPLATFORM', 'firefox'))
        .pipe(gulp.dest('./target/firefox'));
};
function taskcopyallV3Chrome(cb) {
    return gulp.src(['./src_v3/main/*.js', './src_v3/main/*.png', './src_v3/main/*.html'])
        .pipe(replace('EXTENSIONPLATFORM', 'chrome'))
        .pipe(gulp.dest('./target/chrome_v3'))
};
function taskcopyallV3FireFox(cb) {
    return gulp.src(['./src_v3/main/*.js', './src_v3/main/*.png', './src_v3/main/*.html'])
        .pipe(replace('EXTENSIONPLATFORM', 'firefox'))
        .pipe(gulp.dest('./target/firefox_v3'))
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
        .pipe(replace('EXTENSIONPLATFORM', 'chrome'))
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
        .pipe(gulp.dest('./target/edge_v3/_locales'))
        .pipe(gulp.dest('./target/firefox_v3/_locales'));
};

//#region manifest

function taskmanifestdevchromeV3(cb) {
    return gulp.src('./src_v3/manifest/manifest_chrome_v3.json')
        .pipe(rename('manifest.json'))
        .pipe(jeditor(function (manifest) {
            manifest.version = ChromeVersion;
            return manifest;
        }))
        .pipe(gulp.dest('./target/chrome_v3'));
};
function taskmanifestreleasechromeV3() {
    return gulp.src('./src_v3/manifest/manifest_chrome_v3.json')
        .pipe(rename('manifest.json'))
        .pipe(jeditor(function (manifest) {
            manifest.version = ChromeVersion;
            // Release version limite to usage of the extension only in a list of allowed domains
            manifest.content_scripts[0].matches = AllowedDomains;
            manifest.web_accessible_resources[0].matches = AllowedDomains;
            return manifest;
        }))
        .pipe(gulp.dest('./target/chrome_v3'));
};

function taskmanifestdevfirefox(cb) {
    return gulp.src('./src/main/manifest.json')
        .pipe(jeditor(function (manifest) {
            manifest.version = FirefoxVersion;
            manifest.applications = { 'gecko': { 'id': 'beidconnect@bosa.be', 'strict_min_version': '58.0' } };
            return manifest;
        }))
        .pipe(gulp.dest('./target/firefox'));
};
function taskmanifestreleasefirefox() {
    return gulp.src('./src/main/manifest.json')
        .pipe(jeditor(function (manifest) {
            manifest.version = FirefoxVersion;
            manifest.content_scripts[0].matches = AllowedDomains;
            manifest.applications = { 'gecko': { 'id': 'beidconnect@bosa.be', 'strict_min_version': '58.0', "update_url": "https://eid.static.bosa.fgov.be/ffupdate-manifest.json" } };
            return manifest;
        }))
        .pipe(gulp.dest('./target/firefox'));
};

function taskmanifestdevfirefoxV3(cb) {
    return gulp.src('./src_v3/manifest/manifest_firefox_v3.json')
        .pipe(rename('manifest.json'))
        .pipe(jeditor(function (manifest) {
            manifest.version = FirefoxVersion;
            return manifest;
        }))
        .pipe(gulp.dest('./target/firefox_v3'));
};
function taskmanifestreleasefirefoxV3(cb) {
    return gulp.src('./src_v3/manifest/manifest_firefox_v3.json')
        .pipe(rename('manifest.json'))
        .pipe(jeditor(function (manifest) {
            manifest.version = FirefoxVersion;
            // Release version limite to usage of the extension only in a list of allowed domains
            manifest.content_scripts[0].matches = AllowedDomains;
            manifest.web_accessible_resources[0].matches = AllowedDomains;
            return manifest;
        }))
        .pipe(gulp.dest('./target/firefox_v3'));
};

function taskmanifestdevedgeV3(cb) {
    return gulp.src('./src_v3/manifest/manifest_edge_v3.json')
        .pipe(rename('manifest.json'))
        .pipe(jeditor(function (manifest) {
            manifest.version = EdgeVersion;
            return manifest;
        }))
        .pipe(gulp.dest('./target/edge_v3'));
};
function taskmanifestreleaseedgeV3() {
    return gulp.src('./src_v3/manifest/manifest_edge_v3.json')
        .pipe(rename('manifest.json'))
        .pipe(jeditor(function (manifest) {
            manifest.version = EdgeVersion;
            // Release version limite to usage of the extension only in a list of allowed domains
            manifest.content_scripts[0].matches = AllowedDomains;
            manifest.web_accessible_resources[0].matches = AllowedDomains;
            return manifest;
        }))
        .pipe(gulp.dest('./target/edge_v3'));
};

//#region ZIP

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
function taskzipfirefoxV2() {
    return gulp.src('./target/firefox/*')
        .pipe(zip('beidconnect-firefox-ext-v2-' + FirefoxVersion + '.zip'))
        .pipe(gulp.dest('./target'));
};
function taskzipfirefoxV3() {
    return gulp.src('./target/firefox_v3/**')
        .pipe(zip('beidconnect-firefox-ext-v3-' + FirefoxVersion + '.zip'))
        .pipe(gulp.dest('./target'));
};

//#region Main

exports.default = gulp.series(
    taskclean,
    gulp.parallel(
        gulp.series(
            tasklint,
            taskcopyallV2FireFox,
            gulp.parallel(
                gulp.series(
                    taskmanifestdevfirefox,
                    taskzipfirefoxV2
                )
            )
        ),
        gulp.series(
            tasklintV3,
            taskcopyallV3Chrome,
            taskcopyallV3FireFox,
            taskcopyallV3Edge1,
            taskcopyallV3Edge2,
            taskcopyallV3Edge3,
            taskcopyallV3Edge4,
            taskcopylocales,
            gulp.parallel(
                gulp.series(
                    taskmanifestdevchromeV3,
                    taskzipchromeV3
                ),
                gulp.series(
                    taskmanifestdevedgeV3,
                    taskzipedgeV3
                ),
                gulp.series(
                    taskmanifestdevfirefoxV3,
                    taskzipfirefoxV3
                )
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
                    taskmanifestreleasefirefox,
                    taskzipfirefoxV2
                )
            )
        ),
        gulp.series(
            tasklintV3,
            gulp.parallel(
                taskMinifyjsV3Chrome,
                taskMinifyjsV3Edge,
                taskjsV3Firefox
            ),
            taskcopypngV3,
            taskcopylocales,
            gulp.parallel(
                gulp.series(
                    taskmanifestreleasechromeV3,
                    taskzipchromeV3
                ),
                gulp.series(
                    taskmanifestreleaseedgeV3,
                    taskzipedgeV3
                ),
                gulp.series(
                    taskmanifestreleasefirefoxV3,
                    taskzipfirefoxV3
                )
            )
        )
    )
);
