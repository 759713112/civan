



//对象OI属性的磁盘结构
struct object_info_t {

    hobject_t soid;  //对象唯一标识
    eversion_t version, prior_version;
    version_t user_version;
    osd_reqid_t last_reqid;

    uint64_t size;
    utime_t mtime;
    utime_t local_mtime; // local mtime

    // note: these are currently encoded into a total 16 bits; see
    // encode()/decode() for the weirdness.
    typedef enum {
        FLAG_LOST        = 1<<0,
        FLAG_WHITEOUT    = 1<<1, // object logically does not exist
        FLAG_DIRTY       = 1<<2, // object has been modified since last flushed or undirtied
        FLAG_OMAP        = 1<<3, // has (or may have) some/any omap data
        FLAG_DATA_DIGEST = 1<<4, // has data crc
        FLAG_OMAP_DIGEST = 1<<5, // has omap crc
        FLAG_CACHE_PIN   = 1<<6, // pin the object in cache tier
        FLAG_MANIFEST    = 1<<7, // has manifest
        FLAG_USES_TMAP   = 1<<8, // deprecated; no longer used
        FLAG_REDIRECT_HAS_REFERENCE = 1<<9, // has reference
    } flag_t;

    flag_t flags;

    static std::string get_flag_string(flag_t flags) {
        std::string s;
        std::vector<std::string> sv = get_flag_vector(flags);
        for (auto ss : sv) {
            s += std::string("|") + ss;
        }
        if (s.length())
            return s.substr(1);
        return s;
    }
    static std::vector<std::string> get_flag_vector(flag_t flags) {
        std::vector<std::string> sv;
        if (flags & FLAG_LOST)
            sv.insert(sv.end(), "lost");
        if (flags & FLAG_WHITEOUT)
            sv.insert(sv.end(), "whiteout");
        if (flags & FLAG_DIRTY)
            sv.insert(sv.end(), "dirty");
        if (flags & FLAG_USES_TMAP)
            sv.insert(sv.end(), "uses_tmap");
        if (flags & FLAG_OMAP)
            sv.insert(sv.end(), "omap");
        if (flags & FLAG_DATA_DIGEST)
            sv.insert(sv.end(), "data_digest");
        if (flags & FLAG_OMAP_DIGEST)
            sv.insert(sv.end(), "omap_digest");
        if (flags & FLAG_CACHE_PIN)
            sv.insert(sv.end(), "cache_pin");
        if (flags & FLAG_MANIFEST)
            sv.insert(sv.end(), "manifest");
        if (flags & FLAG_REDIRECT_HAS_REFERENCE)
            sv.insert(sv.end(), "redirect_has_reference");
        return sv;
    }
    std::string get_flag_string() const {
        return get_flag_string(flags);
    }

    uint64_t truncate_seq, truncate_size;

    std::map<std::pair<uint64_t, entity_name_t>, watch_info_t> watchers;

    // opportunistic checksums; may or may not be present
    __u32 data_digest;  ///< data crc32c
    __u32 omap_digest;  ///< omap crc32c

    // alloc hint attribute
    uint64_t expected_object_size, expected_write_size;
    uint32_t alloc_hint_flags;

    struct object_manifest_t manifest;

    void copy_user_bits(const object_info_t& other);

    bool test_flag(flag_t f) const {
        return (flags & f) == f;
    }
    void set_flag(flag_t f) {
        flags = (flag_t)(flags | f);
    }
    void clear_flag(flag_t f) {
        flags = (flag_t)(flags & ~f);
    }
    bool is_lost() const {
        return test_flag(FLAG_LOST);
    }
    bool is_whiteout() const {
        return test_flag(FLAG_WHITEOUT);
    }
    bool is_dirty() const {
        return test_flag(FLAG_DIRTY);
    }
    bool is_omap() const {
        return test_flag(FLAG_OMAP);
    }
    bool is_data_digest() const {
        return test_flag(FLAG_DATA_DIGEST);
    }
    bool is_omap_digest() const {
        return test_flag(FLAG_OMAP_DIGEST);
    }
    bool is_cache_pinned() const {
        return test_flag(FLAG_CACHE_PIN);
    }
    bool has_manifest() const {
        return test_flag(FLAG_MANIFEST);
    }
    void set_data_digest(__u32 d) {
        set_flag(FLAG_DATA_DIGEST);
        data_digest = d;
    }
    void set_omap_digest(__u32 d) {
        set_flag(FLAG_OMAP_DIGEST);
        omap_digest = d;
    }
    void clear_data_digest() {
        clear_flag(FLAG_DATA_DIGEST);
        data_digest = -1;
    }
    void clear_omap_digest() {
        clear_flag(FLAG_OMAP_DIGEST);
        omap_digest = -1;
    }
    void new_object() {
        clear_data_digest();
        clear_omap_digest();
    }

    void encode(ceph::buffer::list& bl, uint64_t features) const;
    void decode(ceph::buffer::list::const_iterator& bl);
    void decode(const ceph::buffer::list& bl) {
        auto p = std::cbegin(bl);
        decode(p);
    }

    void dump(ceph::Formatter *f) const;
    static void generate_test_instances(std::list<object_info_t*>& o);

    explicit object_info_t()
    : user_version(0), size(0), flags((flag_t)0),
        truncate_seq(0), truncate_size(0),
        data_digest(-1), omap_digest(-1),
        expected_object_size(0), expected_write_size(0),
        alloc_hint_flags(0)
    {}

    explicit object_info_t(const hobject_t& s)
    : soid(s),
        user_version(0), size(0), flags((flag_t)0),
        truncate_seq(0), truncate_size(0),
        data_digest(-1), omap_digest(-1),
        expected_object_size(0), expected_write_size(0),
        alloc_hint_flags(0)
    {}

    explicit object_info_t(const ceph::buffer::list& bl) {
        decode(bl);
    }

    explicit object_info_t(const ceph::buffer::list& bl, const hobject_t& _soid) {
        decode_no_oid(bl);
        soid = _soid;
    }
};

std::ostream& operator<<(std::ostream& out, const object_info_t& oi);

struct ObjectState {
  object_info_t oi;
  bool exists;         ///< the stored object exists (i.e., we will remember the object_info_t)

  ObjectState() : exists(false) {}

  ObjectState(const object_info_t &oi_, bool exists_)
    : oi(oi_), exists(exists_) {}
  ObjectState(object_info_t &&oi_, bool exists_)
    : oi(std::move(oi_)), exists(exists_) {}
  ObjectState(const hobject_t &obj) : oi(obj), exists(false) {}
};

//对象OI 和 SS属性   属性缓存和读写锁
struct ObjectContext {
  ObjectState obs;

  Context *destructor_callback;

public:

  // any entity in obs.oi.watchers MUST be in either watchers or unconnected_watchers.
  std::map<std::pair<uint64_t, entity_name_t>, WatchRef> watchers;

  // attr cache
  std::map<std::string, ceph::buffer::list, std::less<>> attr_cache;

  RWState rwstate;
  std::list<OpRequestRef> waiters;  ///< ops waiting on state change
  bool get_read(OpRequestRef& op) {
    if (rwstate.get_read_lock()) {
      return true;
    } // else
      // Now we really need to bump up the ref-counter.
    waiters.emplace_back(op);
    rwstate.inc_waiters();
    return false;
  }
  bool get_write(OpRequestRef& op, bool greedy=false) {
    if (rwstate.get_write_lock(greedy)) {
      return true;
    } // else
    if (op) {
      waiters.emplace_back(op);
      rwstate.inc_waiters();
    }
    return false;
  }
  bool get_excl(OpRequestRef& op) {
    if (rwstate.get_excl_lock()) {
      return true;
    } // else
    if (op) {
      waiters.emplace_back(op);
      rwstate.inc_waiters();
    }
    return false;
  }
  void wake(std::list<OpRequestRef> *requeue) {
    rwstate.release_waiters();
    requeue->splice(requeue->end(), waiters);
  }
  void put_read(std::list<OpRequestRef> *requeue) {
    if (rwstate.put_read()) {
      wake(requeue);
    }
  }
  void put_write(std::list<OpRequestRef> *requeue) {
    if (rwstate.put_write()) {
      wake(requeue);
    }
  }
  void put_excl(std::list<OpRequestRef> *requeue) {
    if (rwstate.put_excl()) {
      wake(requeue);
    }
  }
  bool empty() const { return rwstate.empty(); }

  bool get_lock_type(OpRequestRef& op, RWState::State type) {
    switch (type) {
    case RWState::RWWRITE:
      return get_write(op);
    case RWState::RWREAD:
      return get_read(op);
    case RWState::RWEXCL:
      return get_excl(op);
    default:
      ceph_abort_msg("invalid lock type");
      return true;
    }
  }
  bool get_write_greedy(OpRequestRef& op) {
    return get_write(op, true);
  }
  bool get_snaptrimmer_write(bool mark_if_unsuccessful) {
    return rwstate.get_snaptrimmer_write(mark_if_unsuccessful);
  }
  bool get_recovery_read() {
    return rwstate.get_recovery_read();
  }
  bool try_get_read_lock() {
    return rwstate.get_read_lock();
  }
  void drop_recovery_read(std::list<OpRequestRef> *ls) {
    ceph_assert(rwstate.recovery_read_marker);
    put_read(ls);
    rwstate.recovery_read_marker = false;
  }
  void put_lock_type(
    RWState::State type,
    std::list<OpRequestRef> *to_wake,
    bool *requeue_recovery,
    bool *requeue_snaptrimmer) {
    switch (type) {
    case RWState::RWWRITE:
      put_write(to_wake);
      break;
    case RWState::RWREAD:
      put_read(to_wake);
      break;
    case RWState::RWEXCL:
      put_excl(to_wake);
      break;
    default:
      ceph_abort_msg("invalid lock type");
    }
    if (rwstate.empty() && rwstate.recovery_read_marker) {
      rwstate.recovery_read_marker = false;
      *requeue_recovery = true;
    }
    if (rwstate.empty() && rwstate.snaptrimmer_write_marker) {
      rwstate.snaptrimmer_write_marker = false;
      *requeue_snaptrimmer = true;
    }
  }
  bool is_request_pending() {
    return !rwstate.empty();
  }

  ObjectContext()
    : ssc(NULL),
      destructor_callback(0),
      blocked(false), requeue_scrub_on_unblock(false) {}

  ~ObjectContext() {
    ceph_assert(rwstate.empty());
    if (destructor_callback)
      destructor_callback->complete(0);
  }

  void start_block() {
    ceph_assert(!blocked);
    blocked = true;
  }
  void stop_block() {
    ceph_assert(blocked);
    blocked = false;
  }
  bool is_blocked() const {
    return blocked;
  }

  /// in-progress copyfrom ops for this object
  bool blocked:1;
  bool requeue_scrub_on_unblock:1;    // true if we need to requeue scrub on unblock

};



struct OpContext {
    OpRequestRef op;
    osd_reqid_t reqid;
    std::vector<OSDOp> *ops;

    const ObjectState *obs; // Old objectstate

    ObjectState new_obs;  // resulting ObjectState
    //pg_stat_t new_stats;  // resulting Stats
    object_stat_sum_t delta_stats;

    bool modify;          // (force) modification (even if op_t is empty)
    bool user_modify;     // user-visible modification
    bool undirty;         // user explicitly un-dirtying this object
    bool cache_operation;     ///< true if this is a cache eviction
    bool ignore_cache;    ///< true if IGNORE_CACHE flag is std::set
    bool ignore_log_op_stats;  // don't log op stats
    bool update_log_only; ///< this is a write that returned an error - just record in pg log for dup detection
    ObjectCleanRegions clean_regions;

    // side effects
    std::list<std::pair<watch_info_t,bool> > watch_connects; ///< new watch + will_ping flag
    std::list<watch_disconnect_t> watch_disconnects; ///< old watch + send_discon
    std::list<notify_info_t> notifies;
    struct NotifyAck {
      std::optional<uint64_t> watch_cookie;
      uint64_t notify_id;
      ceph::buffer::list reply_bl;
      explicit NotifyAck(uint64_t notify_id) : notify_id(notify_id) {}
      NotifyAck(uint64_t notify_id, uint64_t cookie, ceph::buffer::list& rbl)
	        : watch_cookie(cookie), notify_id(notify_id) {
	    reply_bl = std::move(rbl);
      }
    };
    std::list<NotifyAck> notify_acks;

    uint64_t bytes_written, bytes_read;

    utime_t mtime;
    eversion_t at_version;       // pg's current version pointer
    version_t user_at_version;   // pg's current user version pointer

    /// index of the current subop - only valid inside of do_osd_ops()
    int current_osd_subop_num;
    /// total number of subops processed in this context for cls_cxx_subop_version()
    int processed_subop_count = 0;

    PGTransactionUPtr op_t;
    std::vector<pg_log_entry_t> log;
    std::optional<pg_hit_set_history_t> updated_hset_history;

    interval_set<uint64_t> modified_ranges;
    ObjectContextRef obc;
    ObjectContextRef clone_obc;    // if we created a clone
    ObjectContextRef head_obc;     // if we also update snapset (see trim_object)

    // FIXME: we may want to kill this msgr hint off at some point!
    std::optional<int> data_off = std::nullopt;

    MOSDOpReply *reply;

    PrimaryLogPG *pg;

    int num_read;    ///< count read ops
    int num_write;   ///< count update ops

    mempool::osd_pglog::vector<std::pair<osd_reqid_t, version_t> > extra_reqids;
    mempool::osd_pglog::map<uint32_t, int> extra_reqid_return_codes;

    hobject_t new_temp_oid, discard_temp_oid;  ///< temp objects we should start/stop tracking

    std::list<std::function<void()>> on_applied;
    std::list<std::function<void()>> on_committed;
    std::list<std::function<void()>> on_finish;
    std::list<std::function<void()>> on_success;
    template <typename F>
    void register_on_finish(F &&f) {
      on_finish.emplace_back(std::forward<F>(f));
    }
    template <typename F>
    void register_on_success(F &&f) {
      on_success.emplace_back(std::forward<F>(f));
    }
    template <typename F>
    void register_on_applied(F &&f) {
      on_applied.emplace_back(std::forward<F>(f));
    }
    template <typename F>
    void register_on_commit(F &&f) {
      on_committed.emplace_back(std::forward<F>(f));
    }

    bool sent_reply = false;

    // pending async reads <off, len, op_flags> -> <outbl, outr>
    std::list<std::pair<boost::tuple<uint64_t, uint64_t, unsigned>,
	      std::pair<ceph::buffer::list*, Context*> > > pending_async_reads;
    int inflightreads;
    friend struct OnReadComplete;
    void start_async_reads(PrimaryLogPG *pg);
    void finish_read(PrimaryLogPG *pg);
    bool async_reads_complete() {
      return inflightreads == 0;
    }

    RWState::State lock_type;
    ObcLockManager lock_manager;

    std::map<int, std::unique_ptr<OpFinisher>> op_finishers;

    OpContext(const OpContext& other);
    const OpContext& operator=(const OpContext& other);

    OpContext(OpRequestRef _op, osd_reqid_t _reqid, std::vector<OSDOp>* _ops,
	      ObjectContextRef& obc,
	      PrimaryLogPG *_pg) :
      op(_op), reqid(_reqid), ops(_ops),
      obs(&obc->obs),
      snapset(0),
      new_obs(obs->oi, obs->exists),
      modify(false), user_modify(false), undirty(false), cache_operation(false),
      ignore_cache(false), ignore_log_op_stats(false), update_log_only(false),
      bytes_written(0), bytes_read(0), user_at_version(0),
      current_osd_subop_num(0),
      obc(obc),
      reply(NULL), pg(_pg),
      num_read(0),
      num_write(0),
      sent_reply(false),
      inflightreads(0),
      lock_type(RWState::RWNONE) {
      if (obc->ssc) {
	new_snapset = obc->ssc->snapset;
	snapset = &obc->ssc->snapset;
      }
    }
    OpContext(OpRequestRef _op, osd_reqid_t _reqid,
              std::vector<OSDOp>* _ops, PrimaryLogPG *_pg) :
      op(_op), reqid(_reqid), ops(_ops), obs(NULL), snapset(0),
      modify(false), user_modify(false), undirty(false), cache_operation(false),
      ignore_cache(false), ignore_log_op_stats(false), update_log_only(false),
      bytes_written(0), bytes_read(0), user_at_version(0),
      current_osd_subop_num(0),
      reply(NULL), pg(_pg),
      num_read(0),
      num_write(0),
      inflightreads(0),
      lock_type(RWState::RWNONE) {}
    void reset_obs(ObjectContextRef obc) {
      new_obs = ObjectState(obc->obs.oi, obc->obs.exists);
      if (obc->ssc) {
	new_snapset = obc->ssc->snapset;
	snapset = &obc->ssc->snapset;
      }
    }
    ~OpContext() {
      ceph_assert(!op_t);
      if (reply)
	reply->put();
      for (std::list<std::pair<boost::tuple<uint64_t, uint64_t, unsigned>,
		     std::pair<ceph::buffer::list*, Context*> > >::iterator i =
	     pending_async_reads.begin();
	   i != pending_async_reads.end();
	   pending_async_reads.erase(i++)) {
	delete i->second.second;
      }
    }
    uint64_t get_features() {
      if (op && op->get_req()) {
        return op->get_req()->get_connection()->get_features();
      }
      return -1ull;
    }
  };