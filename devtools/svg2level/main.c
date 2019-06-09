#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlstring.h>
#include <libxml/debugXML.h>

#define RECTS_CAPACITY 1024
#define TEXTS_CAPACITY 1024
#define BUFFER_CAPACITY (640 * 1024)

#define STATIC_ARRAY(type, name, ...)                           \
    type name[] = {__VA_ARGS__};                                \
    const size_t name##_count = sizeof(name) / sizeof(type);

#ifdef LIBXML_TREE_ENABLED

typedef struct {
    xmlNode **rects;
    size_t rects_count;
    xmlNode **texts;
    size_t texts_count;
    char *buffer;
} Context;

static void print_usage(FILE *stream)
{
    fprintf(stream, "Usage: ./svg2level <input.svg...> <output-folder>\n");
}

static void fail_node(xmlNode *node, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    vfprintf(stderr, format, args);
    xmlDebugDumpNode(stderr, node, 0);

    va_end(args);
    exit(-1);
}

static size_t extract_nodes_by_name(xmlNode *root, const char *node_name, xmlNode **rects, size_t n)
{
    if (n == 0) {
        return 0;
    }

    const size_t old_n = n;

    for (xmlNode *iter = root; n > 0 && iter; iter = iter->next) {
        if (iter->type == XML_ELEMENT_NODE && xmlStrEqual(iter->name, (const xmlChar*)node_name)) {
            rects[0] = iter;
            n--;
            rects++;

        }

        const size_t m = extract_nodes_by_name(iter->children, node_name, rects, n);

        n -= m;
        rects += m;
    }

    return old_n - n;
}

static const char *color_of_style(const char *style)
{
    const char *prefix = "fill:#";
    const char *fill = strstr(style, prefix);
    if (fill == NULL) {
        return NULL;
    }
    return fill + strlen(prefix);
}

static xmlNode *find_attr_by_name(xmlNode *node, const char *attr_name)
{
    for (xmlAttr *attr = node->properties; attr; attr = attr->next) {
        if (xmlStrEqual(attr->name, (const xmlChar *)attr_name)) {
            return attr->children;
        }
    }

    return NULL;
}

static xmlNode *require_attr_by_name(xmlNode *node, const char *attr_name)
{
    xmlNode *attr = find_attr_by_name(node, attr_name);
    if (attr == NULL) {
        fail_node(node, "Required attribute `%s`\n", attr_name);
    }
    return attr;
}

static const char *require_color_of_node(xmlNode *node)
{
    xmlNode *style_attr = require_attr_by_name(node, "style");
    const char *color = color_of_style((const char*)style_attr->content);
    if (color == NULL) {
        fail_node(node, "`style` attr does not define the `fill` of the rectangle\n");
    }
    return color;
}

static size_t filter_nodes_by_id_prefix(xmlNode **input, size_t input_count,
                                        const char* id_prefix,
                                        xmlNode **output, size_t output_capacity)
{
    size_t output_count = 0;

    for (size_t i = 0; i < input_count && output_count < output_capacity; ++i) {
        xmlNode *id_attr = require_attr_by_name(input[i], "id");
        if (xmlStrstr(id_attr->content, (const xmlChar*)id_prefix) == id_attr->content) {
            output[output_count++] = input[i];
        }
    }

    return output_count;
}

static xmlNode *find_node_by_id(xmlNode **nodes, size_t n, const char *id)
{
    for (size_t i = 0; i < n; ++i) {
        xmlNode *idAttr = find_attr_by_name(nodes[i], "id");
        if (idAttr != NULL && xmlStrEqual(idAttr->content, (const xmlChar*)id)) {
            return nodes[i];
        }
    }

    return NULL;
}

static xmlNode *require_node_by_id(xmlNode **nodes, size_t n, const char *id)
{
    xmlNode *node = find_node_by_id(nodes, n, id);
    if (node == NULL) {
        fail_node(node, "Could find node with id `%s`\n", id);
    }
    return node;
}

static void save_pack_by_id_prefix(Context *context, FILE *output_file,
                                   const char *id_prefix,
                                   const char **attrs, size_t attrs_count)
{
    xmlNode **pack = (xmlNode**)context->buffer;
    size_t pack_count = filter_nodes_by_id_prefix(
        context->rects, context->rects_count, id_prefix,
        pack, BUFFER_CAPACITY / sizeof(xmlNode*));
    pack_count += filter_nodes_by_id_prefix(
        context->texts, context->texts_count, id_prefix,
        pack + pack_count,
        BUFFER_CAPACITY / sizeof(xmlNode*) - pack_count);

    fprintf(output_file, "%ld\n", pack_count);
    for (size_t i = 0; i < pack_count; ++i) {
        for (size_t j = 0; j < attrs_count; ++j) {
            if (strcmp(attrs[j], "#color") == 0) {
                const char *color = require_color_of_node(pack[i]);
                fprintf(output_file, "%.6s ", color);
            } else {
                xmlNode *attr = require_attr_by_name(pack[i], attrs[j]);
                fprintf(output_file, "%s ", attr->content);
            }
        }
        fprintf(output_file, "\n");
    }
}

////////////////////////////////////////////////////////////

static void save_title(Context *context, FILE *output_file)
{
    xmlNode *node = require_node_by_id(context->texts, context->texts_count, "title");
    for (xmlNode *iter = node->children; iter; iter = iter->next) {
        fprintf(output_file, "%s", iter->children->content);
    }
    fprintf(output_file, "\n");
}

static void save_background(Context *context, FILE *output_file)
{
    fprintf(output_file, "%.6s\n",
            require_color_of_node(
                require_node_by_id(
                    context->rects, context->rects_count,
                    "background")));
}

static size_t read_file_to_buffer(const char *filename,
                                  char *buffer, size_t buffer_capacity)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file `%s`\n", filename);
        exit(-1);
    }
    size_t buffer_count = fread(buffer, sizeof(char), buffer_capacity, file);
    fclose(file);
    return buffer_count;
}

static size_t count_lines(char *buffer, size_t buffer_count)
{
    size_t count = 0;
    for (size_t i = 0; i < buffer_count; ++i) {
        if (buffer[i] == '\n') {
            count++;
        }
    }
    return count;
}

static size_t tokcpy(const char *src, char *dest, size_t dest_capacity)
{
    assert(src);
    assert(dest);

    size_t dest_count = 0;
    while(*src != ' ' && *src != '\0' && (dest_capacity - dest_count) > 0) {
        *dest++ = *src++;
        dest_count++;
    }

    if(dest_capacity - dest_count > 0) {
        *dest++ = '\0';
        dest_count++;
    } else {
        fprintf(stderr, "tokcpy() ran out of memory\n");
        exit(-1);
    }

    return dest_count;
}

static void skip_ws(const char **s)
{
    while (**s == ' ' && **s != '\0') {
        (*s)++;
    }
}

static int next_token(const char **s, size_t *n)
{
    skip_ws(s);

    *n = 0;
    while (*(*s + *n) != '\0' && *(*s + *n) != ' ') {
        (*n)++;
    }

    return *n > 0;
}

static void print_args(FILE *output_file, const char *s)
{
    fprintf(output_file, "(set args '(");

    size_t n = 0;
    while (next_token(&s, &n)) {
        fprintf(output_file, "\"");
        fwrite(s, sizeof(char), n, output_file);
        fprintf(output_file, "\"");
        s += n;
    }

    fprintf(output_file, "))\n");
}

static void save_script(FILE *output_file, xmlNode *scripted,
                        char *buffer, size_t buffer_capacity)
{
    for (xmlNode *iter = scripted->children; iter; iter = iter->next) {
        if (!xmlStrEqual(iter->name, (const xmlChar*)"title")) {
            continue;
        }
        size_t filename_count = tokcpy((const char*)iter->children->content, buffer, buffer_capacity);
        const char *filename = buffer;

        buffer += filename_count;
        buffer_capacity -= filename_count;

        const size_t buffer_count = read_file_to_buffer(
            filename, buffer, buffer_capacity);
        const size_t lines_count = count_lines(buffer, buffer_count);
        fprintf(output_file, "%ld\n", lines_count + 1);
        print_args(output_file, (const char*)iter->children->content + filename_count - 1);
        fwrite(buffer, sizeof(char), buffer_count, output_file);
    }
}

static void save_player(Context *context, FILE *output_file)
{
    xmlNode *node = require_node_by_id(context->rects, context->rects_count, "player");
    const char *color = require_color_of_node(node);
    xmlNode *xAttr = require_attr_by_name(node, "x");
    xmlNode *yAttr = require_attr_by_name(node, "y");
    fprintf(output_file, "%s %s %.6s\n", xAttr->content, yAttr->content, color);
}

static void save_platforms(Context *context, FILE *output_file)
{
    STATIC_ARRAY(const char *, attrs, "id", "x", "y", "width", "height", "#color");
    save_pack_by_id_prefix(context, output_file, "rect", attrs, attrs_count);
}

static void save_goals(Context *context, FILE *output_file)
{
    STATIC_ARRAY(const char *, attrs, "id", "x", "y", "#color");
    save_pack_by_id_prefix(context, output_file, "goal", attrs, attrs_count);
}

static void save_lava(Context *context, FILE *output_file)
{
    STATIC_ARRAY(const char *, attrs, "id", "x", "y", "width", "height", "#color");
    save_pack_by_id_prefix(context, output_file, "lava", attrs, attrs_count);
}

static void save_backplatforms(Context *context, FILE *output_file)
{
    STATIC_ARRAY(const char *, attrs, "id", "x", "y", "width", "height", "#color");
    save_pack_by_id_prefix(context, output_file, "backrect", attrs, attrs_count);
}

static void save_boxes(Context *context, FILE *output_file)
{
    STATIC_ARRAY(const char*, attrs,
                 "id", "x", "y", "width", "height", "#color");
    save_pack_by_id_prefix(context, output_file, "box", attrs, attrs_count);
}

static void save_labels(Context *context, FILE *output_file)
{
    xmlNode **labels = (xmlNode**)context->buffer;
    size_t labels_count = filter_nodes_by_id_prefix(
        context->texts, context->texts_count,
        "label",
        labels, BUFFER_CAPACITY / sizeof(xmlNode*));

    fprintf(output_file, "%ld\n", labels_count);
    for (size_t i = 0; i < labels_count; ++i) {
        xmlNode *id = require_attr_by_name(labels[i], "id");
        xmlNode *x = require_attr_by_name(labels[i], "x");
        xmlNode *y = require_attr_by_name(labels[i], "y");
        const char *color = require_color_of_node(labels[i]);
        fprintf(output_file, "%s %s %s %.6s\n",
                id->content, x->content, y->content, color);
        // TODO(#432): svg2rects doesn't handle newlines in labels
        for (xmlNode *iter = labels[i]->children; iter; iter = iter->next) {
            fprintf(output_file, "%s", iter->children->content);
        }
        fprintf(output_file, "\n");
    }
}

static void save_regions(Context *context, FILE *output_file)
{
    xmlNode **regions = (xmlNode**)context->buffer;
    size_t regions_count = filter_nodes_by_id_prefix(
        context->rects, context->rects_count,
        "script",
        regions, BUFFER_CAPACITY / sizeof(xmlNode*));

    fprintf(output_file, "%ld\n", regions_count);
    for (size_t i = 0; i < regions_count; ++i) {
        xmlNode *id = require_attr_by_name(regions[i], "id");
        xmlNode *x = require_attr_by_name(regions[i], "x");
        xmlNode *y = require_attr_by_name(regions[i], "y");
        xmlNode *width = require_attr_by_name(regions[i], "width");
        xmlNode *height = require_attr_by_name(regions[i], "height");
        const char *color = require_color_of_node(regions[i]);
        fprintf(output_file, "%s %s %s %s %s %.6s\n",
                id->content,
                x->content, y->content,
                width->content, height->content,
                color);
    }
}

static void save_level_script(Context *context, FILE *output_file)
{
    xmlNode *level_script = find_node_by_id(
        context->rects, context->rects_count,
        "level_script");

    if (level_script) {
        save_script(
            output_file,
            level_script,
            context->buffer, BUFFER_CAPACITY);
    } else {
        fprintf(output_file, "0\n");
    }
}

static void save_level(Context *context, FILE *output_file)
{
    save_title(context, output_file);
    save_background(context, output_file);
    save_player(context, output_file);
    save_platforms(context, output_file);
    save_goals(context, output_file);
    save_lava(context, output_file);
    save_backplatforms(context, output_file);
    save_boxes(context, output_file);
    save_labels(context, output_file);
    save_regions(context, output_file);
    save_level_script(context, output_file);
}


static int compile_svg_file(const char *input_filename, const char *output_filename)
{
    xmlDoc *doc = xmlReadFile(input_filename, NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Could not parse file `%s`\n", input_filename);
        return -1;
    }

    xmlNode *root = xmlDocGetRootElement(doc);

    Context context;
    context.rects = calloc(RECTS_CAPACITY, sizeof(xmlNode*));
    context.rects_count = extract_nodes_by_name(root, "rect", context.rects, RECTS_CAPACITY);
    context.texts = calloc(TEXTS_CAPACITY, sizeof(xmlNode*));
    context.texts_count = extract_nodes_by_name(root, "text", context.texts, TEXTS_CAPACITY);
    context.buffer = calloc(BUFFER_CAPACITY, sizeof(char));

    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        fprintf(stderr, "Could not open file `%s`\n", output_filename);
        return -1;
    }

    save_level(&context, output_file);

    fclose(output_file);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    free(context.rects);
    free(context.texts);
    free(context.buffer);

    return 0;
}

static const char *filename(const char *path)
{
    const int n = strlen(path);

    int i = n - 1;
    while (i >= 0 && path[i] != '/') {
        --i;
    }

    return &path[i + 1];
}

static const char *extension(const char *path)
{
    const int n = strlen(path);

    int i = n - 1;
    while (i >= 0 && path[i] != '.') {
        --i;
    }

    return &path[i + 1];
}

static int compile_subcommand(char **input_filenames, size_t n,
                              char *output_folder)
{
    char input_name[1024] = {0};
    char output_filename[1024] = {0};

    sprintf(output_filename, "%s/meta.txt", output_folder);
    FILE *fmeta = fopen(output_filename, "w");
    if (fmeta == NULL) {
        fprintf(stderr, "Could not open file `%s`\n", output_filename);
        return -1;
    }

    for (size_t i = 0; i < n; ++i) {
        memcpy(
            input_name,
            filename(input_filenames[i]),
            extension(input_filenames[i]) - filename(input_filenames[i]) - 1);

        fprintf(fmeta, "%s.txt\n", input_name);

        memset(output_filename, 0, 1024);
        sprintf(output_filename, "%s/%s.txt", output_folder, input_name);

        printf("Compiling %s into %s\n", input_filenames[i], output_filename);

        if (compile_svg_file(input_filenames[i], output_filename) < 0) {
            return -1;
        }
    }

    fclose(fmeta);

    return 0;
}

int main(int argc, char **argv)
{
    LIBXML_TEST_VERSION

    if (argc < 3) {
        print_usage(stderr);
        return -1;
    }

    return compile_subcommand(argv + 1, argc - 2, argv[argc - 1]);
}

#else
int main(void) {
    fprintf(stderr, "Tree support not compiled in\n");
    exit(1);
}
#endif
