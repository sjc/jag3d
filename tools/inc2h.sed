#
# inc2h.sed
# convert a `.inc` file containing definition to a C `.h`
#
# usage:
#   sed -r -n -f inc2h.sed <file>.inc
# -f command file
# -n don't echo lines as they're processed
# -r extended regexs (-E)
#

# convert un-commented `SYMBOL = VALUE` lines to `#define SYMBOL VALUE`
s/^([^;].*)=(.*)/#define\t\1\t\2/

# convert lines starting with `;` to `//` comments
s/^;/\/\//

# convert `;` preceeded by whitespace to `//` comments
s/(\s);/\1\/\//

# print the line with any conversions made
p
