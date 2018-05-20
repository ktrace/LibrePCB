#!/usr/bin/env bash

# set shell settings (see https://sipb.mit.edu/doc/safe-shell/)
set -euv -o pipefail

# detect CI
if [ -n "${TRAVIS-}" ]
then
  IS_PULL_REQUEST="$TRAVIS_PULL_REQUEST"
  BRANCH_NAME="$TRAVIS_BRANCH"
elif [ -n "${APPVEYOR-}" ]
then
  IS_PULL_REQUEST="${APPVEYOR_PULL_REQUEST_NUMBER:-false}"
  BRANCH_NAME="$APPVEYOR_REPO_BRANCH"
fi

# upload build artifacts for all branches of the upstream repository
if [ "${IS_PULL_REQUEST}" = "false" -a -n "${UPLOAD_URL-}" ]
then
  # create archive of the artifacts directory
  7z a -tzip ./artifacts.zip -w ./artifacts/* > /dev/null
  # create digital signature
  openssl dgst -sha256 -sign <(echo -e "$UPLOAD_SIGN_KEY") -out ./artifacts.zip.sha256 ./artifacts.zip
  # create archive containing the artifacts and its digital signature
  BASENAME=$(echo $BRANCH_NAME | sed -e 's/[^A-Za-z0-9_-]/_/g')
  7z a -tzip "./$BASENAME.zip" -w ./artifacts.zip ./artifacts.zip.sha256 > /dev/null
  # upload archive
  curl --fail --basic -u "$UPLOAD_USER:$UPLOAD_PASS" -T "./$BASENAME.zip" "$UPLOAD_URL/$BASENAME.zip"
fi
