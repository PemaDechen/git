/*
 *  --check turns on checking that the working tree matches the
 *    files that are being modified, but doesn't apply the patch
 *  --stat does just a diffstat, and doesn't actually apply
 *  --numstat does numeric diffstat, and doesn't actually apply
 *  --index-info shows the old and new index info for paths if available.
 *  --index updates the cache as well.
 *  --cached updates only the cache without ever touching the working tree.
 */
static const char *prefix;
static int prefix_length = -1;
static int newfd = -1;

static int unidiff_zero;
static int p_value = 1;
static int p_value_known;
static int check_index;
static int update_index;
static int cached;
static int diffstat;
static int numstat;
static int summary;
static int check;
static int apply = 1;
static int apply_in_reverse;
static int apply_with_reject;
static int apply_verbosely;
static int allow_overlap;
static int no_add;
static int threeway;
static int unsafe_paths;
static const char *fake_ancestor;
static int line_termination = '\n';
static unsigned int p_context = UINT_MAX;
static const char * const apply_usage[] = {
	N_("git apply [<options>] [<patch>...]"),
	NULL
};

static enum ws_error_action {
} ws_error_action = warn_on_ws_error;
static int whitespace_error;
static int squelch_whitespace_errors = 5;
static int applied_after_fixing_ws;
static enum ws_ignore {
} ws_ignore_action = ignore_ws_none;
static const char *patch_input_file;
static struct strbuf root = STRBUF_INIT;
static int read_stdin = 1;
static int options;
static void parse_whitespace_option(const char *option)
		ws_error_action = warn_on_ws_error;
		ws_error_action = warn_on_ws_error;
		ws_error_action = nowarn_ws_error;
		ws_error_action = die_on_ws_error;
		ws_error_action = die_on_ws_error;
		squelch_whitespace_errors = 0;
		ws_error_action = correct_ws_error;
static void parse_ignorewhitespace_option(const char *option)
		ws_ignore_action = ignore_ws_none;
		ws_ignore_action = ignore_ws_change;
static void set_default_whitespace_mode(const char *whitespace_option)
	if (!whitespace_option && !apply_default_whitespace)
		ws_error_action = (apply ? warn_on_ws_error : nowarn_ws_error);
/*
 * For "diff-stat" like behaviour, we keep track of the biggest change
 * we've seen, and the longest filename. That allows us to do simple
 * scaling.
 */
static int max_change, max_len;

/*
 * Various "current state", notably line numbers and what
 * file (and how) we're patching right now.. The "is_xxxx"
 * things are flags, where -1 means "don't know yet".
 */
static int linenr = 1;

/*
 * Records filenames that have been touched, in order to handle
 * the case where more than one patches touch the same file.
 */

static struct string_list fn_table;

static char *find_name_gnu(const char *line, const char *def, int p_value)
	if (root.len)
		strbuf_insert(&name, 0, root.buf, root.len);
static char *find_name_common(const char *line, const char *def,
			      int p_value, const char *end, int terminate)
	if (root.len) {
		char *ret = xstrfmt("%s%.*s", root.buf, len, start);
static char *find_name(const char *line, char *def, int p_value, int terminate)
		char *name = find_name_gnu(line, def, p_value);
	return find_name_common(line, def, p_value, NULL, terminate);
static char *find_name_traditional(const char *line, char *def, int p_value)
		char *name = find_name_gnu(line, def, p_value);
		return find_name_common(line, def, p_value, NULL, TERM_TAB);
	return find_name_common(line, def, p_value, line + len, 0);
static int guess_p_value(const char *nameline)
	name = find_name_traditional(nameline, NULL, 0);
	else if (prefix) {
		if (!strncmp(name, prefix, prefix_length))
			val = count_slashes(prefix);
			if (!strncmp(cp, prefix, prefix_length))
				val = count_slashes(prefix) + 1;
static void parse_traditional_patch(const char *first, const char *second, struct patch *patch)
	if (!p_value_known) {
		p = guess_p_value(first);
		q = guess_p_value(second);
			p_value = p;
			p_value_known = 1;
		name = find_name_traditional(second, NULL, p_value);
		name = find_name_traditional(first, NULL, p_value);
		first_name = find_name_traditional(first, NULL, p_value);
		name = find_name_traditional(second, first_name, p_value);
		die(_("unable to find filename in patch at line %d"), linenr);
static int gitdiff_hdrend(const char *line, struct patch *patch)
static char *gitdiff_verify_name(const char *line, int isnull, char *orig_name, int side)
	if (!orig_name && !isnull)
		return find_name(line, NULL, p_value, TERM_TAB);
	if (orig_name) {
		int len = strlen(orig_name);
			    orig_name, linenr);
		another = find_name(line, NULL, p_value, TERM_TAB);
		if (!another || memcmp(another, orig_name, len + 1))
			    _("git apply: bad git-diff - inconsistent old filename on line %d"), linenr);
		return orig_name;
			die(_("git apply: bad git-diff - expected /dev/null on line %d"), linenr);
		return NULL;
static int gitdiff_oldname(const char *line, struct patch *patch)
	patch->old_name = gitdiff_verify_name(line, patch->is_new, patch->old_name,
					      DIFF_OLD_NAME);
static int gitdiff_newname(const char *line, struct patch *patch)
	patch->new_name = gitdiff_verify_name(line, patch->is_delete, patch->new_name,
					      DIFF_NEW_NAME);
static int gitdiff_oldmode(const char *line, struct patch *patch)
static int gitdiff_newmode(const char *line, struct patch *patch)
static int gitdiff_delete(const char *line, struct patch *patch)
	return gitdiff_oldmode(line, patch);
static int gitdiff_newfile(const char *line, struct patch *patch)
	return gitdiff_newmode(line, patch);
static int gitdiff_copysrc(const char *line, struct patch *patch)
	patch->old_name = find_name(line, NULL, p_value ? p_value - 1 : 0, 0);
static int gitdiff_copydst(const char *line, struct patch *patch)
	patch->new_name = find_name(line, NULL, p_value ? p_value - 1 : 0, 0);
static int gitdiff_renamesrc(const char *line, struct patch *patch)
	patch->old_name = find_name(line, NULL, p_value ? p_value - 1 : 0, 0);
static int gitdiff_renamedst(const char *line, struct patch *patch)
	patch->new_name = find_name(line, NULL, p_value ? p_value - 1 : 0, 0);
static int gitdiff_similarity(const char *line, struct patch *patch)
static int gitdiff_dissimilarity(const char *line, struct patch *patch)
static int gitdiff_index(const char *line, struct patch *patch)
static int gitdiff_unrecognized(const char *line, struct patch *patch)
static const char *skip_tree_prefix(const char *line, int llen)
	if (!p_value)
	nslash = p_value;
static char *git_header_name(const char *line, int llen)
		cp = skip_tree_prefix(first.buf, first.len);
			cp = skip_tree_prefix(sp.buf, sp.len);
		cp = skip_tree_prefix(second, line + llen - second);
	name = skip_tree_prefix(line, llen);
			np = skip_tree_prefix(sp.buf, sp.len);
			second = skip_tree_prefix(name + len + 1,
static int parse_git_header(const char *line, int len, unsigned int size, struct patch *patch)
	patch->def_name = git_header_name(line, len);
	if (patch->def_name && root.len) {
		char *s = xstrfmt("%s%s", root.buf, patch->def_name);
	linenr++;
	for (offset = len ; size > 0 ; offset += len, size -= len, line += len, linenr++) {
			int (*fn)(const char *, struct patch *);
			if (p->fn(line + oplen, patch) < 0)
static int find_header(const char *line, unsigned long size, int *hdrsize, struct patch *patch)
	for (offset = 0; size > 0; offset += len, size -= len, line += len, linenr++) {
			    linenr, (int)len-1, line);
			int git_hdr_len = parse_git_header(line, len, size, patch);
					       p_value),
					    p_value, linenr);
				    "(line %d)", linenr);
		parse_traditional_patch(line, line+len, patch);
		linenr += 2;
static void record_ws_error(unsigned result, const char *line, int len, int linenr)
	whitespace_error++;
	if (squelch_whitespace_errors &&
	    squelch_whitespace_errors < whitespace_error)
		patch_input_file, linenr, err, len, line);
static void check_whitespace(const char *line, int len, unsigned ws_rule)
	record_ws_error(result, line + 1, len - 2, linenr);
static int parse_fragment(const char *line, unsigned long size,
			  struct patch *patch, struct fragment *fragment)
	linenr++;
	     offset += len, size -= len, line += len, linenr++) {
			if (!apply_in_reverse &&
			    ws_error_action == correct_ws_error)
				check_whitespace(line, len, patch->ws_rule);
			if (apply_in_reverse &&
			    ws_error_action != nowarn_ws_error)
				check_whitespace(line, len, patch->ws_rule);
			if (!apply_in_reverse &&
			    ws_error_action != nowarn_ws_error)
				check_whitespace(line, len, patch->ws_rule);
static int parse_single_patch(const char *line, unsigned long size, struct patch *patch)
		fragment->linenr = linenr;
		len = parse_fragment(line, size, patch, fragment);
			die(_("corrupt patch at line %d"), linenr);
static struct fragment *parse_binary_hunk(char **buf_p,
	linenr++;
		linenr++;
	      linenr-1, llen-1, buffer);
static int parse_binary(char *buffer, unsigned long size, struct patch *patch)
	forward = parse_binary_hunk(&buffer, &size, &status, &used);
		return error(_("unrecognized binary patch at line %d"), linenr-1);
	reverse = parse_binary_hunk(&buffer, &size, &status, &used_1);
static void prefix_one(char **name)
	*name = xstrdup(prefix_filename(prefix, prefix_length, *name));
static void prefix_patch(struct patch *p)
	if (!prefix || p->is_toplevel_relative)
	prefix_one(&p->new_name);
	prefix_one(&p->old_name);
static struct string_list limit_by_name;
static int has_include;
static void add_name_limit(const char *name, int exclude)
	it = string_list_append(&limit_by_name, name);
static int use_patch(struct patch *p)
	if (0 < prefix_length) {
		if (pathlen <= prefix_length ||
		    memcmp(prefix, pathname, prefix_length))
	for (i = 0; i < limit_by_name.nr; i++) {
		struct string_list_item *it = &limit_by_name.items[i];
	return !has_include;
static int parse_chunk(char *buffer, unsigned long size, struct patch *patch)
	int offset = find_header(buffer, size, &hdrsize, patch);
	prefix_patch(patch);
	if (!use_patch(patch))
	patchsize = parse_single_patch(buffer + offset + hdrsize,
				       size - offset - hdrsize, patch);
			linenr++;
			used = parse_binary(buffer + hd + llen,
					linenr++;
		if ((apply || check) &&
			die(_("patch with only garbage at line %d"), linenr);
static void show_stats(struct patch *patch)
	max = max_len;
	max = max + max_change > 70 ? 70 - max : max_change;
	if (max_change > 0) {
		int total = ((add + del) * max + max_change / 2) / max_change;
		add = (add * max + max_change / 2) / max_change;
		size_t len = postimage->line[i].len;
			memmove(new, old, len);
			old += len;
			new += len;
		old += len;
		len = preimage->line[ctx].len;
		memcpy(new, fixed, len);
		new += len;
		fixed += len;
		postimage->line[i].len = len;
static int match_fragment(struct image *img,
	} else if (ws_error_action == correct_ws_error &&
	if (ws_ignore_action == ignore_ws_change) {
		size_t imgoff = 0;
		size_t preoff = 0;
		size_t postlen = postimage->len;
		size_t extra_chars;
		char *preimage_eof;
		char *preimage_end;
		for (i = 0; i < preimage_limit; i++) {
			size_t prelen = preimage->line[i].len;
			size_t imglen = img->line[try_lno+i].len;

			if (!fuzzy_matchlines(img->buf + try + imgoff, imglen,
					      preimage->buf + preoff, prelen))
				return 0;
			if (preimage->line[i].flag & LINE_COMMON)
				postlen += imglen - prelen;
			imgoff += imglen;
			preoff += prelen;
		}
		/*
		 * Ok, the preimage matches with whitespace fuzz.
		 *
		 * imgoff now holds the true length of the target that
		 * matches the preimage before the end of the file.
		 *
		 * Count the number of characters in the preimage that fall
		 * beyond the end of the file and make sure that all of them
		 * are whitespace characters. (This can only happen if
		 * we are removing blank lines at the end of the file.)
		 */
		buf = preimage_eof = preimage->buf + preoff;
		for ( ; i < preimage->nr; i++)
			preoff += preimage->line[i].len;
		preimage_end = preimage->buf + preoff;
		for ( ; buf < preimage_end; buf++)
			if (!isspace(*buf))
				return 0;

		/*
		 * Update the preimage and the common postimage context
		 * lines to use the same whitespace as the target.
		 * If whitespace is missing in the target (i.e.
		 * if the preimage extends beyond the end of the file),
		 * use the whitespace from the preimage.
		 */
		extra_chars = preimage_end - preimage_eof;
		strbuf_init(&fixed, imgoff + extra_chars);
		strbuf_add(&fixed, img->buf + try, imgoff);
		strbuf_add(&fixed, preimage_eof, extra_chars);
		fixed_buf = strbuf_detach(&fixed, &fixed_len);
		update_pre_post_images(preimage, postimage,
				fixed_buf, fixed_len, postlen);
		return 1;
	}

	if (ws_error_action != correct_ws_error)
static int find_pos(struct image *img,
		if (match_fragment(img, preimage, postimage,
static void update_image(struct image *img,
	if (!allow_overlap)
static int apply_one_fragment(struct image *img, struct fragment *frag,
		if (apply_in_reverse) {
			if (first == '+' && no_add)
			    !whitespace_error ||
			    ws_error_action != correct_ws_error) {
				ws_fix_copy(&newlines, patch + 1, plen, ws_rule, &applied_after_fixing_ws);
			if (apply_verbosely)
			   (frag->oldpos == 1 && !unidiff_zero));
	match_end = !unidiff_zero && !trailing;
		applied_pos = find_pos(img, &preimage, &postimage, pos,
		if ((leading <= p_context) && (trailing <= p_context))
		    ws_error_action != nowarn_ws_error) {
			record_ws_error(WS_BLANK_AT_EOF, "+", 1,
			if (ws_error_action == correct_ws_error) {
			if (ws_error_action == die_on_ws_error)
				apply = 0;
		if (apply_verbosely && applied_pos != pos) {
			if (apply_in_reverse)
		update_image(img, applied_pos, &preimage, &postimage);
		if (apply_verbosely)
static int apply_binary_fragment(struct image *img, struct patch *patch)
	if (apply_in_reverse) {
static int apply_binary(struct image *img, struct patch *patch)
		if (apply_binary_fragment(img, patch))
static int apply_fragments(struct image *img, struct patch *patch)
		return apply_binary(img, patch);
		if (apply_one_fragment(img, frag, inaccurate_eof, ws_rule, nth)) {
			if (!apply_with_reject)
static struct patch *in_fn_table(const char *name)
	item = string_list_lookup(&fn_table, name);
static void add_to_fn_table(struct patch *patch)
		item = string_list_insert(&fn_table, patch->new_name);
		item = string_list_insert(&fn_table, patch->old_name);
static void prepare_fn_table(struct patch *patch)
			item = string_list_insert(&fn_table, patch->old_name);
static struct patch *previous_patch(struct patch *patch, int *gone)
	previous = in_fn_table(patch->old_name);
static int load_patch_target(struct strbuf *buf,
	if (cached || check_index) {
static int load_preimage(struct image *image,
	previous = previous_patch(patch, &status);
		status = load_patch_target(&buf, ce, st,
static int load_current(struct image *image, struct patch *patch)
	status = load_patch_target(&buf, ce, &st, name, mode);
static int try_threeway(struct image *image, struct patch *patch,
			struct stat *st, const struct cache_entry *ce)
	if (apply_fragments(&tmp_image, patch) < 0) {
		if (load_current(&tmp_image, patch))
		if (load_preimage(&tmp_image, patch, st, ce))
static int apply_data(struct patch *patch, struct stat *st, const struct cache_entry *ce)
	if (load_preimage(&image, patch, st, ce) < 0)
	    apply_fragments(&image, patch) < 0) {
		if (!threeway || try_threeway(&image, patch, st, ce) < 0)
	add_to_fn_table(patch);
static int check_preimage(struct patch *patch, struct cache_entry **ce, struct stat *st)
	previous = previous_patch(patch, &status);
	} else if (!cached) {
	if (check_index && !previous) {
		if (!cached && verify_index_match(*ce, st))
		if (cached)
	if (!cached && !previous)
static int check_to_create(const char *new_name, int ok_if_exists)
	if (check_index &&
	if (cached)
/*
 * We need to keep track of how symlinks in the preimage are
 * manipulated by the patches.  A patch to add a/b/c where a/b
 * is a symlink should not be allowed to affect the directory
 * the symlink points at, but if the same patch removes a/b,
 * it is perfectly fine, as the patch removes a/b to make room
 * to create a directory a/b so that a/b/c can be created.
 */
static struct string_list symlink_changes;
#define SYMLINK_GOES_AWAY 01
#define SYMLINK_IN_RESULT 02

static uintptr_t register_symlink_changes(const char *path, uintptr_t what)
	ent = string_list_lookup(&symlink_changes, path);
		ent = string_list_insert(&symlink_changes, path);
static uintptr_t check_symlink_changes(const char *path)
	ent = string_list_lookup(&symlink_changes, path);
static void prepare_symlink_changes(struct patch *patch)
			register_symlink_changes(patch->old_name, SYMLINK_GOES_AWAY);
			register_symlink_changes(patch->new_name, SYMLINK_IN_RESULT);
static int path_is_beyond_symlink_1(struct strbuf *name)
		change = check_symlink_changes(name->buf);
		if (check_index) {
static int path_is_beyond_symlink(const char *name_)
	ret = path_is_beyond_symlink_1(&name);
static int check_patch(struct patch *patch)
	status = check_preimage(patch, &ce, &st);
	if ((tpatch = in_fn_table(new_name)) &&
		int err = check_to_create(new_name, ok_if_exists);
		if (err && threeway) {
	if (!unsafe_paths)
	if (!patch->is_delete && path_is_beyond_symlink(patch->new_name))
	if (apply_data(patch, &st, ce) < 0)
static int check_patch_list(struct patch *patch)
	prepare_symlink_changes(patch);
	prepare_fn_table(patch);
		if (apply_verbosely)
		err |= check_patch(patch);
static void stat_patch_list(struct patch *patch)
		show_stats(patch);
static void numstat_patch_list(struct patch *patch)
		write_name_quoted(name, stdout, line_termination);
static void patch_stats(struct patch *patch)
	if (lines > max_change)
		max_change = lines;
		if (len > max_len)
			max_len = len;
		if (len > max_len)
			max_len = len;
static void remove_file(struct patch *patch, int rmdir_empty)
	if (update_index) {
	if (!cached) {
static void add_index_file(const char *path, unsigned mode, void *buf, unsigned long size)
	if (!update_index)
		if (!cached) {
static void create_one_file(char *path, unsigned mode, const char *buf, unsigned long size)
	if (cached)
static void add_conflicted_stages_file(struct patch *patch)
	if (!update_index)
static void create_file(struct patch *patch)
	create_one_file(path, mode, buf, size);
		add_conflicted_stages_file(patch);
		add_index_file(path, mode, buf, size);
static void write_out_one_result(struct patch *patch, int phase)
			remove_file(patch, 1);
			create_file(patch);
		remove_file(patch, patch->is_rename);
		create_file(patch);
static int write_out_one_reject(struct patch *patch)
		if (apply_verbosely)
static int write_out_results(struct patch *list)
				write_out_one_result(l, phase);
					if (write_out_one_reject(l))
static int apply_patch(int fd, const char *filename, int options)
	patch_input_file = filename;
		nr = parse_chunk(buf.buf + offset, buf.len - offset, patch);
		if (apply_in_reverse)
		if (use_patch(patch)) {
			patch_stats(patch);
			if (apply_verbosely)
	if (whitespace_error && (ws_error_action == die_on_ws_error))
		apply = 0;
	update_index = check_index && apply;
	if (update_index && newfd < 0)
		newfd = hold_locked_index(&lock_file, 1);
	if (check_index) {
	if ((check || apply) &&
	    check_patch_list(list) < 0 &&
	    !apply_with_reject)
	if (apply && write_out_results(list)) {
		if (apply_with_reject)
	if (fake_ancestor)
		build_fake_ancestor(list, fake_ancestor);
	if (diffstat)
		stat_patch_list(list);
	if (numstat)
		numstat_patch_list(list);
	if (summary)
	string_list_clear(&fn_table, 0);
	add_name_limit(arg, 1);
	add_name_limit(arg, 0);
	has_include = 1;
			  const char *arg, int unset)
	p_value = atoi(arg);
	p_value_known = 1;
			  const char *arg, int unset)
		ws_ignore_action = ignore_ws_none;
		ws_ignore_action = ignore_ws_change;
	const char **whitespace_option = opt->value;

	*whitespace_option = arg;
	parse_whitespace_option(arg);
	strbuf_reset(&root);
	strbuf_addstr(&root, arg);
	strbuf_complete(&root, '/');
int cmd_apply(int argc, const char **argv, const char *prefix_)
	int is_not_gitdir = !startup_info->have_repository;

	const char *whitespace_option = NULL;
		{ OPTION_CALLBACK, 0, "exclude", NULL, N_("path"),
		{ OPTION_CALLBACK, 0, "include", NULL, N_("path"),
		{ OPTION_CALLBACK, 'p', NULL, NULL, N_("num"),
		OPT_BOOL(0, "no-add", &no_add,
		OPT_BOOL(0, "stat", &diffstat,
		OPT_BOOL(0, "numstat", &numstat,
		OPT_BOOL(0, "summary", &summary,
		OPT_BOOL(0, "check", &check,
		OPT_BOOL(0, "index", &check_index,
		OPT_BOOL(0, "cached", &cached,
		OPT_BOOL(0, "unsafe-paths", &unsafe_paths,
		OPT_BOOL('3', "3way", &threeway,
		OPT_FILENAME(0, "build-fake-ancestor", &fake_ancestor,
		OPT_SET_INT('z', NULL, &line_termination,
		OPT_INTEGER('C', NULL, &p_context,
		{ OPTION_CALLBACK, 0, "whitespace", &whitespace_option, N_("action"),
		{ OPTION_CALLBACK, 0, "ignore-space-change", NULL, NULL,
		{ OPTION_CALLBACK, 0, "ignore-whitespace", NULL, NULL,
		OPT_BOOL('R', "reverse", &apply_in_reverse,
		OPT_BOOL(0, "unidiff-zero", &unidiff_zero,
		OPT_BOOL(0, "reject", &apply_with_reject,
		OPT_BOOL(0, "allow-overlap", &allow_overlap,
		OPT__VERBOSE(&apply_verbosely, N_("be verbose")),
		{ OPTION_CALLBACK, 0, "directory", NULL, N_("root"),
	prefix = prefix_;
	prefix_length = prefix ? strlen(prefix) : 0;
	git_apply_config();
	if (apply_default_whitespace)
		parse_whitespace_option(apply_default_whitespace);
	if (apply_default_ignorewhitespace)
		parse_ignorewhitespace_option(apply_default_ignorewhitespace);
	argc = parse_options(argc, argv, prefix, builtin_apply_options,
	if (apply_with_reject && threeway)
		die("--reject and --3way cannot be used together.");
	if (cached && threeway)
		die("--cached and --3way cannot be used together.");
	if (threeway) {
		if (is_not_gitdir)
			die(_("--3way outside a repository"));
		check_index = 1;
	}
	if (apply_with_reject)
		apply = apply_verbosely = 1;
	if (!force_apply && (diffstat || numstat || summary || check || fake_ancestor))
		apply = 0;
	if (check_index && is_not_gitdir)
		die(_("--index outside a repository"));
	if (cached) {
		if (is_not_gitdir)
			die(_("--cached outside a repository"));
		check_index = 1;
	}
	if (check_index)
		unsafe_paths = 0;
	for (i = 0; i < argc; i++) {
		const char *arg = argv[i];
		int fd;
		if (!strcmp(arg, "-")) {
			errs |= apply_patch(0, "<stdin>", options);
			read_stdin = 0;
			continue;
		} else if (0 < prefix_length)
			arg = prefix_filename(prefix, prefix_length, arg);
		fd = open(arg, O_RDONLY);
		if (fd < 0)
			die_errno(_("can't open patch '%s'"), arg);
		read_stdin = 0;
		set_default_whitespace_mode(whitespace_option);
		errs |= apply_patch(fd, arg, options);
		close(fd);
	}
	set_default_whitespace_mode(whitespace_option);
	if (read_stdin)
		errs |= apply_patch(0, "<stdin>", options);
	if (whitespace_error) {
		if (squelch_whitespace_errors &&
		    squelch_whitespace_errors < whitespace_error) {
			int squelched =
				whitespace_error - squelch_whitespace_errors;
			warning(Q_("squelched %d whitespace error",
				   "squelched %d whitespace errors",
				   squelched),
				squelched);
		}
		if (ws_error_action == die_on_ws_error)
			die(Q_("%d line adds whitespace errors.",
			       "%d lines add whitespace errors.",
			       whitespace_error),
			    whitespace_error);
		if (applied_after_fixing_ws && apply)
			warning("%d line%s applied after"
				" fixing whitespace errors.",
				applied_after_fixing_ws,
				applied_after_fixing_ws == 1 ? "" : "s");
		else if (whitespace_error)
			warning(Q_("%d line adds whitespace errors.",
				   "%d lines add whitespace errors.",
				   whitespace_error),
				whitespace_error);
	}

	if (update_index) {
		if (write_locked_index(&the_index, &lock_file, COMMIT_LOCK))
			die(_("Unable to write new index file"));
	}

	return !!errs;