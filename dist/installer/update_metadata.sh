#!/usr/bin/env bash

# set shell settings (see https://sipb.mit.edu/doc/safe-shell/)
set -euf -o pipefail

# get absolute path to installer root directory
ROOT="$( cd "$(dirname "$0")" ; pwd -P )"

# get parameters
TARGET_NAME="$1"
APP_VERSION_FULL="$2"
APP_VERSION_SHORT="${APP_VERSION_FULL%.0}"  # remove patch version if it is zero
COMMIT_COUNT="`git -C "$ROOT" rev-list --count HEAD`"
DATE="`date +%Y-%m-%d`"

echo "Updating installer metadata for $TARGET_NAME v$APP_VERSION_FULL..."

# replace specific variable in a given file
function replace {
  FILE="$1"
  FROM="$2"
  TO="$3"
  sed -i -e "s/$FROM/$TO/g" "$FILE"
}

# go through all *.xml files
for f in $(find "$ROOT" -name '*.xml')
do
  echo "Update $f..."
  replace "$f" "TARGET_NAME"            "$TARGET_NAME"
  replace "$f" "APP_VERSION_FULL"       "$APP_VERSION_FULL"
  replace "$f" "APP_VERSION_SHORT"      "$APP_VERSION_SHORT"
  replace "$f" "APP_VERSION_NIGHTLY"    "$APP_VERSION_FULL.$COMMIT_COUNT"
  replace "$f" "RELEASE_DATE"           "$DATE"
done
