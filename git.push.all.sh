#!/bin/sh

for a in origin sourcehut codeberg ; do
  echo $a
  git push $a
  git push $a --tags 
  echo
done

#echo Gitlab annoyance: I had to use Chrome to update my SSH key
#git push gitlab
#echo

cat > /dev/null << EOF
[remote "origin"]
        url = git@github.com:samboy/lunacy.git
        fetch = +refs/heads/*:refs/remotes/origin/*
[remote "sourcehut"]
        url = git@git.sr.ht:~samiam/Lunacy
[remote "codeberg"]
        url = git@codeberg.org:samboy/lunacy.git
EOF

