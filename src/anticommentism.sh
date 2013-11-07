#!/bin/sh
echo 'For more information on anticommentism, visit http://www.anticommentist.org/'
grep '/\*' *.c *.h || exit 0
echo 'This is an anticommentist project.  No code comments will be committed.'
exit 1
