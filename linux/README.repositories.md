Building Linux repositories
===========================

There are three types of repositories: Debian repositories (used by
Debian and Ubuntu, where the repository index files are signed but
individual packages aren't), Fedora RPM repositories (used by Fedora and
RHEL/CentOS, where individual packages are signed but repository index
files aren't), and Suse RPM repositories (used by OpenSUSE, where both
the repository index files and individual packages are signed)

This document explains how to build them.

All of them require a GPG key. To generate, install gnupg, and run:

    gpg --full-generate-key

which will ask a few questions and then generate a keypair. Answer the
questions as follows:

- Kind of key: RSA and RSA
- Key size: 4096 bits
- Validity time: 0 (key does not expire). Confirm.
- Real name: "Belgium eIDLink signing key".
- Email address: (leave empty, enter nothing).
- Comment: "official" for the key that will be used for officially
  released packages, or something else (your pick) for a key for
  packages built from continuous integration or for testing.
- After entering these details, enter "O" to confirm generating the
  key.
- GnuPG will now ask for a passphrase. For the official key, enter one;
  for a key that is to be used by continuous integration, do not enter
  one. If no key is entered, GnuPG will ask for confirmation that you
  really do not want to enter a passphrase. Choose the "yes" option.

After generating a key, GnuPG will output the detailed information of
the key, like so:

    pub   rsa4096 2020-12-21 [SC]
          71AF16C7790BF60D8AAC8DFC7FDCDC78C37061C9
    uid                      Belgium eIDLink signing key (official)
    sub   rsa4096 2020-12-21 [E]

The long hexadecimal string on the second line of the output is the
key's fingerprint, and this will be different from key to key. Note it
down for future reference (alternatively, you can look it up again by
running `gpg --list-secret-keys`).

Export the secret key by running `gpg --export-secret-key --armor
--output secretkey.asc`, and store the file `secretkey.asc` which that
creates in a secure location for backup purposes. Note that the export
is still encrypted to the passphrase (if any) that was entered before.

After generating the GnuPG keys, we are ready to create the
repositories.

Debian repositories
-------------------

Debian repositories are most easily created and maintained by way of the
`reprepro` tool, which can be installed on any Debian or Ubuntu system.

First, create the directory structure that reprepro needs:

    mkdir -p /srv/repo/reprepro/conf
    mkdir -p /srv/repo/reprepro/incoming
    cd /srv/repo/reprepro/conf

Next, configure the known distributions:

    cat > /srv/repo/reprepro/conf/distributions <<EOF
    Codename: buster
    Origin: BOSA
    Label: BOSA
    Description: BOSA eIDLink packages for Debian 10 "Buster" - official releases
    Architectures: source amd64 i386
    Components: main
    SignWith: 71AF16C7790BF60D8AAC8DFC7FDCDC78C37061C9
    EOF

Replace the value of SignWith with your GPG key fingerprint. Add more
stanzas (separated from the first by way of an empty line) for other
Debian or Ubuntu distributions, as needed.

Next, create the "incoming" configuration:

    cat > /srv/repo/reprepro/conf/incoming <<EOF
    Name: incoming
    IncomingDir: incoming
    LogDir: logs
    TempDir: /tmp
    Multiple: Yes
    Permit: older_version unused_files
    Allow: buster
    EOF

If more distributions were added to the `distributions` file, add the
value(s) of the `Codename:` line(s) in the relevant stanza(s) of the
`distributions` file to the "Allow" line in the `incoming` file.

Finally, set the default general options for reprepro:

    cat > /srv/repo/reprepro/conf/options
    morguedir +o/old-versions
    outdir +b/repository

To add files to the repository, copy the relevant `.changes` file to
`/srv/repo/reprepro/incoming`, along with all the files mentioned in the
`.changes` file. The `dcmd` command from the `devscripts` package may
help with this. Then, ask reprepro to process the files:

    cd /srv/repo/reprepro
    reprepro processincoming incoming

If the used GnuPG key has a passphrase set, it will ask you to enter it
when the repository index files' signatures are created.

For more information on the possible actions that can be performed by
reprepro, read the man page: `man reprepro`.

Make sure that the contents of the "repository" directory which the
reprepro command above creates, is made available, over HTTP, to your
users.

Fedora RPM repositories
-----------------------

The tool to manage Fedora RPM repositories is at a somewhat lower level
than `reprepro` is. In order to create a repository, one must place
files in the correct directory, and then run the `createrepo` tool on
the correct directory.

To do so, run the correct commands:

    mkdir -p /srv/repo/rpm/fedora/33/RPMS
    cp *.rpm /srv/repo/rpm/fedora/33/RPMS
    cd /srv/repo/rpm/fedora/33/RPMS
    rpmsign --resign *.rpm
    cd ..
    createrepo .

If more distributions are required, then repeat the above commands
(substituting the correct RPM files in the `cp` command) for the other
distributions.

The RPM repositories created in this manner can be exported over HTTP to
the user.

The necessary `createrepo` and `rpmsign` tools are available on all
supported Linux distributions, including Debian and Ubuntu.

Suse RPM repositories
---------------------

The only difference between a Suse repository and a Fedora repository is
that the Suse repository contains signed metadata; the metadata format
is exactly the same as the Fedora metadata format.

In order to create a Suse repository, first create the repository using
the exact same procedure as the Fedora RPM repositories, above:

    mkdir -p /srv/repo/rpm/suse/15.2/RPMS
    cp *.rpm /srv/repo/rpm/suse/15.2/RPMS
    cd /srv/repo/rpm/suse/15.2/RPMS
    rpmsign --resign *.rpm
    cd ..
    createrepo .

Then, sign the metadata:

    cd repodata
    gpg --export --armor 71AF16C7790BF60D8AAC8DFC7FDCDC78C37061C9 --output repomd.xml.key
    gpg --clearsign --detach-sign repomd.xml

Note, the second line contains `--export`, *not* `--export-secret-key`.
The HTTP export should not contain the secret key, only the public key!

Archive packages
================

To build `eidlink-archive` packages that can be used to configure users'
systems, the [eid-archive
package](https://github.com/Fedict/eid-archive) is a good basis to start
from.
