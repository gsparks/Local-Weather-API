
/*
Little XML library for C

Make XML tags with XML_tag()

XML my_xml = XML_tag("tag-name",
	"attr-name-1", "attr-value-1",
	"attr-name-2", "attr-value-2",
	NULL,  // Done with attributes
	"Some text & stuff in the tag",
	XML_tag("child-tag",
		NULL,  // No attributes
		NULL  // No children
	),
	NULL // Done with children
)
As you see, you give the name of the tag as the first argument, followed by
pairs of strings for each attribute and each value.  Then give NULL to signify
the end of the attributes.  Then give the children of the tag, which are
either strings (plain text outside of tags) or more XML tags.  Finish with
NULL to signify the end of children.  Do not escape ampersands, angle brackets,
and quotes in the strings you give to this function.

You can turn the XML into a string with XML_as_text()
const char* text = XML_as_text(my_xml);
which give you a string containing:
<tag-name attr-name-1="attr-value-1" attr-name-2="attr-value-2">Some text &amp; stuff in the tag<child-tag/></tag-name>


You can find a tag that is a child of another tag by name with XML_get_child()
XML child = XML_get_child(my_xml, "child-tag")  // Yields <child-tag/>

You can get the value of an attribute of a tag by name with XML_get_attr()
const char* val = XML_get_attr(my_xml, "attr-name-2")  // Yields "attr-value-2"


You can parse an XML string with XML_parse()
XML parsed = XML_parse("<wwxtp><query><command>TEST</command><position lat=\"23.01515\" long=\"-15.132\"/></query></wwxtp>");
After that, you must check that the parse succeeded.
if (!XML_is_valid(parsed)) {
	fprintf(stderr, "Syntax error in XML.\n");
	send_error_message();
}


BUGS: Giving an empty string as one of the children in XML_tag will confuse
 the parser, since it'll think it's an XML tag.  It's not possible to work
 around this without changing the interface to something less user-friendly.


*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <gc/gc.h>
#include <ctype.h>
<<<<<<< HEAD
typedef unsigned int uint;
typedef union XML XML;

typedef struct XML_Attr {
	const char* name;
	const char* value;
} XML_Attr;

typedef struct XML_Tag {
	uint is_str;
	const char* name;
	uint n_attrs;
	XML_Attr* attrs;
	uint n_contents;
	XML* contents;
} XML_Tag;

union XML {
	XML_Tag* tag;
	const char* str;
=======

#include "xml.h"


typedef struct XML_str_replace_info {
    const char* from;
    const char* to;
    uint from_len;
    uint to_len;
} XML_str_replace_info;
typedef struct XML_strpos_link XML_strpos_link;
struct XML_strpos_link {
    uint pos;
    XML_str_replace_info* info;
    XML_strpos_link* tail;
>>>>>>> 728d49efa6826ac1ff9ec42b6b2e1e152c693cc5
};
const char* XML_str_replace (const char* from, uint n, ...) {
    uint fi, ti, ri;
    va_list args;
    va_start(args, n);
    XML_str_replace_info* rep_infos = GC_malloc(n * sizeof(XML_str_replace_info));
    for (ri = 0; ri < n; ri++) {
        rep_infos[ri].from = va_arg(args, const char*);
        rep_infos[ri].to = va_arg(args, const char*);
        rep_infos[ri].from_len = strlen(rep_infos[ri].from);
        rep_infos[ri].to_len = strlen(rep_infos[ri].to);
    }
    va_end(args);
    XML_strpos_link* reps = NULL;
    XML_strpos_link** cur_reps = &reps;
    int len_diff = 0;
    for (fi = 0; from[fi]; fi++) {
        for (ri = 0; ri < n; ri++) {
            if (0==memcmp(from+fi, rep_infos[ri].from, rep_infos[ri].from_len)) {
                *cur_reps = GC_malloc(sizeof(XML_strpos_link));
                (*cur_reps)->pos = fi;
                (*cur_reps)->info = &rep_infos[ri];
                (*cur_reps)->tail = NULL;
                cur_reps = &(*cur_reps)->tail;
                len_diff += rep_infos[ri].to_len - rep_infos[ri].from_len;
                fi += rep_infos[ri].from_len;
                break;  // from looping over rep_infos
            }
        }
    }
    char* to = GC_malloc(fi + len_diff + 1);
    for (fi = 0, ti = 0; reps; reps = reps->tail) {
        memcpy(to+ti, from+fi, reps->pos - fi);
        ti += reps->pos - fi;
        fi = reps->pos;
        memcpy(to+ti, reps->info->to, reps->info->to_len);
        ti += reps->info->to_len;
        fi += reps->info->from_len;
    }
    strcpy(to+ti, from+fi);
    return (const char*)to;
}


uint XML_is_str (XML xml) { return xml.tag->is_str; }
uint XML_is_valid (XML xml) { return xml.tag != NULL; }

uint XML_strlen (XML xml) {
	uint r = 0;
	if (XML_is_str(xml)) {
		uint i;
		for (i = 0; xml.str[i]; i++) {
			switch (xml.str[i]) {
				case '<':
				case '>': { r += 4; break; }  // &lt; &gt;
				case '&': { r += 5; break; }  // &amp;
				case '"': { r += 6; break; }  // &quot;
				default: { r += 1; break; }
			}
		}
		return r;
	}
	else if (xml.tag->n_contents) {  // <tag></tag>
		r = 5;
		r += 2 * strlen(xml.tag->name);
		uint i;
		for (i = 0; i < xml.tag->n_contents; i++) {
			r += XML_strlen(xml.tag->contents[i]);
		}
	}
	else {  // <tag/>
		r = 3;
		r += strlen(xml.tag->name);
	}
	uint i;
	for (i = 0; i < xml.tag->n_attrs; i++) {
		r += 4;  // (space)name="value"
		r += strlen(xml.tag->attrs[i].name);
		r += XML_strlen((XML)xml.tag->attrs[i].value);
	}
	return r;
}

const char* XML_escape (const char* in) {
    return XML_str_replace(in, 4,
        "<", "&lt;",
        ">", "&gt;",
        "&", "&amp;",
        "\"", "&quot;"
    );
}

const char* XML_unescape (const char* in) {
    return XML_str_replace(in, 4,
        "&lt;", "<",
        "&gt;", ">",
        "&amp;", "&",
        "&quot;", "\""
    );
}

const char* XML_as_text (XML xml) {
	if (XML_is_str(xml)) {
		return XML_escape(xml.str);
	}
	else {
		char* r = GC_malloc(XML_strlen(xml) + 1);
		uint ri = 0;
		r[ri++] = '<';
		uint i;
		uint namelen = strlen(xml.tag->name);
		memcpy(r+ri, xml.tag->name, namelen);
		ri += namelen;
		for (i = 0; i < xml.tag->n_attrs; i++) {
			r[ri++] = ' ';
			uint attrnamelen = strlen(xml.tag->attrs[i].name);
			memcpy(r+ri, xml.tag->attrs[i].name, attrnamelen);
			ri += attrnamelen;
			r[ri++] = '=';
			r[ri++] = '"';
			const char* attrvalue = XML_escape(xml.tag->attrs[i].value);
			uint attrvaluelen = strlen(attrvalue);
			memcpy(r+ri, attrvalue, attrvaluelen);
			ri += attrvaluelen;
			r[ri++] = '"';
		}
		if (xml.tag->n_contents) {
			r[ri++] = '>';
			for (i = 0; i < xml.tag->n_contents; i++) {
				const char* content = XML_as_text(xml.tag->contents[i]);
				uint contentlen = strlen(content);
				memcpy(r+ri, content, contentlen);
				ri += contentlen;
			}
			r[ri++] = '<';
			r[ri++] = '/';
			memcpy(r+ri, xml.tag->name, namelen);
			ri += namelen;
			r[ri++] = '>';
		}
		else {
			r[ri++] = '/';
			r[ri++] = '>';
		}
		r[ri] = 0;
		return r;
	}
}


XML XML_tag (const char* name, ...) {
	va_list args;
	va_start(args, name);
	XML_Tag* r = GC_malloc(sizeof(XML_Tag));
    r->is_str = 0;
	r->name = name;
	r->n_attrs = 0;
    r->attrs = GC_malloc(0);
    const char* attr_name;
	while (attr_name = va_arg(args, const char*)) {
        r->attrs = GC_realloc(r->attrs, (r->n_attrs+1) * sizeof(XML_Attr));
        r->attrs[r->n_attrs].name = attr_name;
        if (!(r->attrs[r->n_attrs].value = va_arg(args, const char*))) {
            fprintf(stderr, "XML error: odd number of strings given in attribute list\n");
            exit(1);
        }
        r->n_attrs++;
    }
    r->n_contents = 0;
    r->contents = GC_malloc(0);
    XML content;
	while (content.tag = (XML_Tag*)va_arg(args, void*)) {
        r->contents = GC_realloc(r->contents, (r->n_contents+1) * sizeof(XML));
        r->contents[r->n_contents] = content;
        r->n_contents++;
    }
	va_end(args);
	return (XML)r;
}

const char* XML_get_attr (XML xml, const char* name) {
	uint i;
	for (i = 0; i < xml.tag->n_attrs; i++)
	if (0==strcmp(xml.tag->attrs[i].name, name))
		return xml.tag->attrs[i].value;
	return NULL;
}
XML XML_get_child (XML xml, const char* name) {
	uint i;
	for (i = 0; i < xml.tag->n_contents; i++)
	if (!XML_is_str(xml.tag->contents[i]))
	if (0==strcmp(xml.tag->contents[i].tag->name, name))
		return xml.tag->contents[i];
	return (XML)(XML_Tag*)NULL;
}

uint XML_isnamechar (char c) {
	return c && c != '>' && c != '/' && c != '"' && c != '=' && !isspace(c);
}
uint XML_isntnamechar (char c) { return !XML_isnamechar(c); }
uint XML_isquote (char c) { return c == '"'; }
uint XML_islt (char c) { return c == '<'; }
const char* XML_extract_until (const char** pp, uint (* f ) (char)) {
	uint i = 0;
	while ((*pp)[i] && !f((*pp)[i])) i++;
	if (!f((*pp)[i])) return NULL;
	char* r = GC_malloc(i + 1);
	memcpy(r, *pp, i);
	r[i] = 0;
	*pp += i;
	return (const char*)r;
}
const char* XML_extract_name (const char** pp) { return XML_extract_until(pp, XML_isntnamechar); }
void XML_eatws (const char** pp) { while (isspace(**pp)) (*pp)++; }

const char* failp = 0;
uint failspot = 0;
XML XML_parse_tag (const char** pp) {
	const char* p = *pp;
	if (*p++ != '<') goto ERR_NEW;
	XML_eatws(&p);
	if (!*p) goto ERR_NEW;
	const char* name = XML_extract_name(&p);
	if (!name || !strlen(name)) goto ERR_NEW;
	XML_eatws(&p);
	uint n_attrs = 0;
	XML_Attr* attrs = GC_malloc(0);
	while (XML_isnamechar(*p)) {
		const char* attrname = XML_extract_name(&p);
		if (!attrname || !strlen(attrname)) goto ERR_NEW;
		XML_eatws(&p);
		if (*p++ != '=') goto ERR_NEW;
		XML_eatws(&p);
		if (*p++ != '"') goto ERR_NEW;
		const char* attrvalesc = XML_extract_until(&p, XML_isquote);
		if (!attrvalesc) goto ERR_NEW;
		if (*p++ != '"') goto ERR_NEW;
		const char* attrval = XML_unescape(attrvalesc);
		attrs = GC_realloc(attrs, (n_attrs + 1) * sizeof(XML_Attr));
		attrs[n_attrs].name = attrname;
		attrs[n_attrs].value = attrval;
		n_attrs++;
		XML_eatws(&p);
		if (!*p) goto ERR_NEW;
	}
	if (*p == '/') {
		p++;
		XML_eatws(&p);
		if (*p++ != '>') goto ERR_NEW;
		XML_Tag* r = GC_malloc(sizeof(XML_Tag));
		r->is_str = 0;
		r->name = name;
		r->n_attrs = n_attrs;
		r->attrs = attrs;
		r->n_contents = 0;
		r->contents = NULL;
		*pp = p;
		return (XML)r;
	}
	else if (*p == '>') {
		p++;
		uint n_contents = 0;
		XML* contents = GC_malloc(0);
		if (!*p) goto ERR_NEW;
		for (;;) {
			if (*p == '<') {
				const char* tagp = p;
				p++;
				XML_eatws(&p);
				if (*p == '/') {
					p++;
					XML_eatws(&p);
					uint i;
					uint namelen = strlen(name);
					for (i = 0; i < namelen; i++)
					if (*p++ != name[i])
						goto ERR_NEW;
					XML_eatws(&p);
					if (*p++ != '>') goto ERR_NEW;
					XML_Tag* r = GC_malloc(sizeof(XML_Tag));
					r->is_str = 0;
					r->name = name;
					r->n_attrs = n_attrs;
					r->attrs = attrs;
					r->n_contents = n_contents;
					r->contents = contents;
					*pp = p;
					return (XML)r;
				}
				else {
					p = tagp;
					XML child = XML_parse_tag(&p);
					if (!XML_is_valid(child)) goto ERR_PROP;
					contents = GC_realloc(contents, (n_contents + 1) * sizeof(XML));
					contents[n_contents] = child;
					n_contents++;
				}
			}
			else {
				const char* textesc = XML_extract_until(&p, XML_islt);
				if (!textesc) goto ERR_NEW;
				const char* text = XML_unescape(textesc);
				contents = GC_realloc(contents, (n_contents + 1) * sizeof(XML));
				contents[n_contents] = (XML)text;
				n_contents++;
			}
		}
	}
	else goto ERR_NEW;
	ERR_NEW:
		failp = p;
	ERR_PROP:
		return (XML)(XML_Tag*)NULL;
}
XML XML_parse (const char* p) {
	XML r = XML_parse_tag(&p);
	failspot = failp - p;
	if (*p) return (XML)(XML_Tag*)NULL;
	else return r;
}
XML XML_parse_n (const char* p, uint n) {
	char* realp = GC_malloc(n + 1);
	memcpy(realp, p, n);
	realp[n] = 0;
	return XML_parse((const char*)realp);
}



void XML_test () {
	XML my_xml = XML_tag("tag-name",
		"attr-name-1", "attr-value-1",
		"attr-name-2", "attr-value-2",
		NULL,  // Done with attributes
		"Some text & stuff in the tag",
		XML_tag("child-tag",
			NULL,  // No attributes
			NULL  // No children
		),
		NULL // Done with children
	);
	puts(XML_as_text(my_xml));
	XML child = XML_get_child(my_xml, "child-tag");  // Yields <child-tag/>
	puts(XML_as_text(child));
	const char* val = XML_get_attr(my_xml, "attr-name-2");  // Yields "attr-value-2"
	puts(val);
	XML parsed = XML_parse("<wwxtp><query><command>TEST</command><position lat=\"23.01515\" long=\"-15.132\"/></query></wwxtp>");
	if (!XML_is_valid(parsed)) {
		fprintf(stderr, "Error: Parse failed at position %u\n", failspot);
		exit(1);
	}
    const char* esct = "asdf<fdsa>asdf&fdsa\"asdf";
    puts(esct);
    esct = XML_escape(esct);
    puts(esct);
    esct = XML_unescape(esct);
    puts(esct);
	puts(XML_as_text(parsed));
}
/*
int main () {
	GC_init();
	XML_test();
	return 0;
}
*/
