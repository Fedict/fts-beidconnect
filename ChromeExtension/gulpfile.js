/*
  Gulp build file for BeIDConnect Chrome Extension.

  To install Node modules: run "npm install"
  To build: run "gulp"
  - targetDebug folder will contain the extension for Chrome, Firefox and Edge without domain filtering
  - targetRelease folder will contain the extension for Chrome, Firefox and Edge with domain filtering
  - targetReleaseWithMinFin folder will contain the extension for Chrome, Firefox and Edge with domain filtering and MinFin domains

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
    rename = require('gulp-rename');

const ChromeVersion = "1.0.4";
const EdgeVersion = "1.0.4";
const FirefoxVersion = "0.0.17";
// Safari version is coded inside the \SafariAppExtension\SafariAppExtension Extension\SafariExtensionHandler.mm source file

// https://developer.chrome.com/docs/extensions/develop/concepts/match-patterns
const AllowedDomains = ['https://*.belgium.be/*', 'https://*.fgov.be/*', 'https://gcloudbelgium.sharepoint.com/*'];
const AllowedDomainsWithMinFin = ['https://*.belgium.be/*', 'https://*.fgov.be/*', 'https://gcloudbelgium.sharepoint.com/*', "https://*.finbel.intra/*", 'https://*.fed.be/*', 'https://*.yourict.be/*'];

var DestDirectory = "./targetDebug";
var DestDomains = AllowedDomains;

//#region clean && parameters

function taskStartDebugSeq(cb) {
    console.log('--- Start Generating Debug Version ---');
    DestDirectory = "./targetDebug";
    del(['targetDebug']);
    DestDomains = AllowedDomains;
    cb();
}

function taskStartReleaseSeq(cb) {
    console.log('--- Start Generating Release Version ---');
    DestDirectory = "./targetRelease";
    del(['targetRealease']);
    DestDomains = AllowedDomains;
    cb();
}

function taskStartReleaseWithMinFinSeq(cb) {
    console.log('--- Start Generating Release Version (with MinFin) ---');
    DestDirectory = "./targetReleaseWithMinFin";
    del(['targetReleaseWithMinFin']);
    DestDomains = AllowedDomainsWithMinFin;
    cb();
}

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
        .pipe(gulp.dest(DestDirectory + '/firefox'))
};
function taskMinifyjsV3Chrome(cb) {
    return gulp.src('./src_v3/main/*.js')
        .pipe(replace('EXTENSIONPLATFORM', 'chrome'))
        .pipe(uglify())
        .pipe(gulp.dest(DestDirectory + '/chrome_v3'))
};
function taskMinifyjsV3Edge(cb) {
    return gulp.src('./src_v3/main/*.js')
        .pipe(replace('EXTENSIONPLATFORM', 'edge'))
        .pipe(uglify())
        .pipe(gulp.dest(DestDirectory + '/edge_v3'));
};
function taskjsV3Firefox(cb) {
    return gulp.src('./src_v3/main/*.js')
        .pipe(replace('EXTENSIONPLATFORM', 'firefox'))
        .pipe(gulp.dest(DestDirectory + '/firefox_v3'));
};

function taskcopypng(cb) {
    return gulp.src(['./src/main/*.png'])
        .pipe(gulp.dest(DestDirectory + '/firefox'));
};

function taskcopypngV3(cb) {
    return gulp.src(['./src_v3/main/*.png'])
        .pipe(gulp.dest(DestDirectory + '/chrome_v3'))
        .pipe(gulp.dest(DestDirectory + '/firefox_v3'))
        .pipe(gulp.dest(DestDirectory + '/edge_v3'));
};

function taskcopyallV2FireFox(cb) {
    return gulp.src(['./src/main/*.js', './src/main/*.png', './src/main/*.html'])
        .pipe(replace('EXTENSIONPLATFORM', 'firefox'))
        .pipe(gulp.dest(DestDirectory + '/firefox'));
};
function taskcopyallV3Chrome(cb) {
    return gulp.src(['./src_v3/main/*.js', './src_v3/main/*.png', './src_v3/main/*.html'])
        .pipe(replace('EXTENSIONPLATFORM', 'chrome'))
        .pipe(gulp.dest(DestDirectory + '/chrome_v3'))
};
function taskcopyallV3FireFox(cb) {
    return gulp.src(['./src_v3/main/*.js', './src_v3/main/*.png', './src_v3/main/*.html'])
        .pipe(replace('EXTENSIONPLATFORM', 'firefox'))
        .pipe(gulp.dest(DestDirectory + '/firefox_v3'))
};
function taskcopyallV3Edge1(cb) {
    return gulp.src(['./src_v3/main/*.png', './src_v3/main/*.html'])
        .pipe(gulp.dest(DestDirectory + '/edge_v3'));
};
function taskcopyallV3Edge2(cb) {
    return gulp.src(['./src_v3/main/content.js',])
        .pipe(replace('beidconnect.page', 'beidconnectedge.page'))
        .pipe(gulp.dest(DestDirectory + '/edge_v3'));
};
function taskcopyallV3Edge3(cb) {
    return gulp.src('./src_v3/main/background.js')
        .pipe(replace('beidconnect.page', 'beidconnectedge.page'))
        .pipe(replace('beidconnect.background', 'beidconnectedge.background'))
        .pipe(replace('EXTENSIONPLATFORM', 'chrome'))
        .pipe(gulp.dest(DestDirectory + '/edge_v3'));
};
function taskcopyallV3Edge4(cb) {
    return gulp.src('./src_v3/main/page.js')
        .pipe(replace('beidconnect.page', 'beidconnectedge.page'))
        .pipe(replace('beidconnect.background', 'beidconnectedge.background'))
        .pipe(gulp.dest(DestDirectory + '/edge_v3'));
};

function taskcopylocales(cb) {
    return gulp.src(['./src_v3/main/_locales/**/*'])
        .pipe(gulp.dest(DestDirectory + '/chrome_v3/_locales'))
        .pipe(gulp.dest(DestDirectory + '/edge_v3/_locales'))
        .pipe(gulp.dest(DestDirectory + '/firefox_v3/_locales'));
};

//#region manifest

function taskmanifestdevchromeV3(cb) {
    return gulp.src('./src_v3/manifest/manifest_chrome_v3.json')
        .pipe(rename('manifest.json'))
        .pipe(jeditor(function (manifest) {
            manifest.version = ChromeVersion;
            return manifest;
        }))
        .pipe(gulp.dest(DestDirectory + '/chrome_v3'));
};
function taskmanifestreleasechromeV3() {
    return gulp.src('./src_v3/manifest/manifest_chrome_v3.json')
        .pipe(rename('manifest.json'))
        .pipe(jeditor(function (manifest) {
            manifest.version = ChromeVersion;
            // Release version limite to usage of the extension only in a list of allowed domains
            manifest.content_scripts[0].matches = DestDomains;
            manifest.web_accessible_resources[0].matches = DestDomains;
            return manifest;
        }))
        .pipe(gulp.dest(DestDirectory + '/chrome_v3'));
};

function taskmanifestdevfirefox(cb) {
    return gulp.src('./src/main/manifest.json')
        .pipe(jeditor(function (manifest) {
            manifest.version = FirefoxVersion;
            manifest.applications = { 'gecko': { 'id': 'beidconnect@bosa.be', 'strict_min_version': '58.0' } };
            return manifest;
        }))
        .pipe(gulp.dest(DestDirectory + '/firefox'));
};
function taskmanifestreleasefirefox() {
    return gulp.src('./src/main/manifest.json')
        .pipe(jeditor(function (manifest) {
            manifest.version = FirefoxVersion;
            manifest.content_scripts[0].matches = DestDomains;
            manifest.applications = { 'gecko': { 'id': 'beidconnect@bosa.be', 'strict_min_version': '58.0', "update_url": "https://eid.static.bosa.fgov.be/ffupdate-manifest.json" } };
            return manifest;
        }))
        .pipe(gulp.dest(DestDirectory + '/firefox'));
};

function taskmanifestdevfirefoxV3(cb) {
    return gulp.src('./src_v3/manifest/manifest_firefox_v3.json')
        .pipe(rename('manifest.json'))
        .pipe(jeditor(function (manifest) {
            manifest.version = FirefoxVersion;
            return manifest;
        }))
        .pipe(gulp.dest(DestDirectory + '/firefox_v3'));
};
function taskmanifestreleasefirefoxV3(cb) {
    return gulp.src('./src_v3/manifest/manifest_firefox_v3.json')
        .pipe(rename('manifest.json'))
        .pipe(jeditor(function (manifest) {
            manifest.version = FirefoxVersion;
            // Release version limite to usage of the extension only in a list of allowed domains
            manifest.content_scripts[0].matches = DestDomains;
            manifest.web_accessible_resources[0].matches = DestDomains;
            return manifest;
        }))
        .pipe(gulp.dest(DestDirectory + '/firefox_v3'));
};

function taskmanifestdevedgeV3(cb) {
    return gulp.src('./src_v3/manifest/manifest_edge_v3.json')
        .pipe(rename('manifest.json'))
        .pipe(jeditor(function (manifest) {
            manifest.version = EdgeVersion;
            return manifest;
        }))
        .pipe(gulp.dest(DestDirectory + '/edge_v3'));
};
function taskmanifestreleaseedgeV3() {
    return gulp.src('./src_v3/manifest/manifest_edge_v3.json')
        .pipe(rename('manifest.json'))
        .pipe(jeditor(function (manifest) {
            manifest.version = EdgeVersion;
            // Release version limite to usage of the extension only in a list of allowed domains
            manifest.content_scripts[0].matches = DestDomains;
            manifest.web_accessible_resources[0].matches = DestDomains;
            return manifest;
        }))
        .pipe(gulp.dest(DestDirectory + '/edge_v3'));
};

//#region ZIP

function taskzipchromeV3() {
    return gulp.src(DestDirectory + '/chrome_v3/**')
        .pipe(zip('beidconnect-chrome-ext-v3-' + ChromeVersion + '.zip'))
        .pipe(gulp.dest(DestDirectory));
};
function taskzipedgeV3() {
    return gulp.src(DestDirectory + '/edge_v3/**')
        .pipe(zip('beidconnect-Edge-ext-v3-' + EdgeVersion + '.zip'))
        .pipe(gulp.dest(DestDirectory));
};
function taskzipfirefoxV2() {
    return gulp.src(DestDirectory + '/firefox/*')
        .pipe(zip('beidconnect-firefox-ext-v2-' + FirefoxVersion + '.zip'))
        .pipe(gulp.dest(DestDirectory));
};
function taskzipfirefoxV3() {
    return gulp.src(DestDirectory + '/firefox_v3/**')
        .pipe(zip('beidconnect-firefox-ext-v3-' + FirefoxVersion + '.zip'))
        .pipe(gulp.dest(DestDirectory));
};

//#region Main

exports.default =
    gulp.series(
        gulp.series(
            taskStartDebugSeq,
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
        ),
        gulp.series(
            taskStartReleaseSeq,
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
        ),
        gulp.series(
            taskStartReleaseWithMinFinSeq,
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
        )
    );
