import os
import copy
import fnmatch


class Fn():

    def __init__(self, name, has_sect, order, oid, win, wout):
        self.name_ = name
        self.has_sect_ = has_sect
        self.order_ = order
        self.oid_ = oid
        self.win_ = win
        self.wout_ = wout
        self.al_ = None

    def add_alignment(self, al):
        assert al >= 0 and al <= 12
        self.al_ = al

    def alignment_directive(self):
        if self.al_ is None:
            return ""
        return " # a={}".format(self.al_)

    def order_line(self):
        return "{}{}".format(self.name_, self.alignment_directive())


class Order_File():

    def __init__(self, fname):
        self.fname = fname

        self.okay = False
        self.order = []
        self.comms = set()
        self.dsos = set()
        self.dsos_bases = set()
        self.dsos_pure_bases = set()
        self.me = set()
        self.total_win_ = 0.0
        try:
            for line in open(self.fname):
                line = line.split(",")
                if len(line) == 4:
                    print("Using old order file. Please regen!")
                    line.append(0)
                    line.append(0)
                    line.append(0)
                if len(line) != 7:
                    continue
                line[0] = line[0].lstrip().rstrip()
                if line[0] == "self":
                    self.me.add(line[1].lstrip().rstrip())
                elif line[0] == "commuse":
                    self.comms.add(line[1].lstrip().rstrip())
                elif line[0] == "dsodep":
                    dsodep = line[1].lstrip().rstrip()
                    self.dsos.add(dsodep)
                    self.dsos_bases.add(os.path.basename(dsodep))
                    self.dsos_pure_bases.add(
                        os.path.basename(dsodep).split(".so")[0])
                elif line[0] == "order":

                    has_sect = line[1].lstrip().rstrip() == "1"
                    name = line[2].lstrip().rstrip()
                    order = line[3].lstrip().rstrip()
                    oid = line[4].lstrip().rstrip()
                    win = line[5].lstrip().rstrip()
                    wout = line[6].lstrip().rstrip()
                    try:
                        order = int(order)
                        oid = int(oid)
                        win = float(win)
                        wout = float(wout)
                        self.total_win_ += win
                        self.okay = True
                        self.order.append(
                            Fn(name, has_sect, order, oid, win, wout))
                    except Exception:
                        print("Bad Line: {}".format(line))
                elif line[0] == "buildid":
                    continue
                else:
                    continue
        except Exception as e:
            print("IO Error: {}".format(e))
            self.okay = False
        if len(self.me) == 0:
            self.okay = False

    def all_weights_in(self):
        return [fn.win_ for fn in self.order]

    def add_alignment_to_hottest_n_per_dso(self, N, al):
        olen = float(len(self.order))
        olen *= N
        cnt = 0
        for fn in sorted(self.order, reverse=True, key=lambda x: x.win_):
            olen -= 1.0
            if olen <= 0.0:
                break
            cnt += 1
            fn.add_alignment(al)
        return cnt

    def add_alignment_to_hottest_n_global(self, bound, al):
        cnt = 0
        for fn in sorted(self.order, reverse=True, key=lambda x: x.win_):
            if fn.win_ < bound:
                break
            cnt += 1
            fn.add_alignment(al)
        return cnt

    def add_alignment_till(self, N, al):
        wp = 0.0
        cnt = 0
        for fn in sorted(self.order, reverse=True, key=lambda x: x.wout_):
            if wp >= N:
                break
            cnt += 1
            wp += fn.wout_
            fn.add_alignment(al)
        return cnt

    def raw_func_order(self):
        raw_order = []
        for f in self.order:
            raw_order.append(f.name_)
        return raw_order

    def merge(self, other):
        assert False
        if not self.okay:
            return copy.deepcopy(other)
        if not other.okay:
            return copy.deepcopy(self)

        new = copy.deepcopy(self)

        new.order += other.order
        new.comms |= other.comms
        new.dsos |= other.dsos
        new.dsos_bases |= other.dsos_bases
        new.dsos_pure_bases |= other.dsos_pure_bases
        new.me |= other.me

    def output_ordering_file(self):
        if not self.okay:
            return None
        out = [
            "commuse_dsodep_buildid_self_or_order," + "name_or_has_section," +
            "section_or_func_name," + "ordering," + "id," +
            "incoming_weight," + "outgoing_weight"
        ]

        for dso in self.me:
            out.append("self,{},,,,,".format(self.dso))
        for dso in self.dsos:
            out.append("dsodep,{},,,,,".format(dso))
        for comm in self.comms:
            out.append("commuse,{},,,,,".format(comm))
        i = 0
        uniques = set()
        for fn in self.order:
            if fn[1] in uniques:
                continue
            uniques.add(fn[1])
            v = "0"
            if fn[0]:
                v = "1"
            out.append("order,{},{},{},{},{},{}".format(
                v, fn.name_, i, fn.oid_, fn.win_, fn.wout_))
            i += 1
        return "\n".join(out)

    def filesize(self):
        return os.path.getsize(self.fname)

    def script_name(self, target):
        return "__and__".join(self.me).replace("/", "__").replace(
            "[", "__").replace("]", "__").replace("(", "__").replace(
                ")", "__") + "." + target

    def key(self):
        return self.fname

    def should_skip_dso(self, dso=None):
        if dso is None:
            dso = self.me
        if not isinstance(dso, set):
            dso = set(dso)
        for d in dso:
            d = os.path.basename(d)
            if d in {
                    "libm.so.6", "linux-vdso.so.1", "libc.so.6",
                    "ld-linux-x86-64.so.2"
            }:
                return True
        return False

    def extract_dso_deps(self, recurse):
        if recurse == "0":
            return set()

        out = set()
        for dso in self.dsos:
            if self.should_skip_dso(dso):
                if recurse == "1":
                    continue
            out.add(dso)
        return out

    def matches_comm(self, comm):
        if not self.okay:
            return False

        if comm in self.comms:
            return True
        for c in self.comms:
            if fnmatch.fnmatchcase(c, comm):
                return True
        return False

    def matches_dso(self, dso):
        if not self.okay:
            return False

        if (dso in self.dsos) or (dso in self.dsos_bases) or (
                dso in self.dsos_pure_bases):
            return True

        all_dsos = self.dsos | self.dsos_bases | self.dsos_pure_bases
        for d in all_dsos:
            if fnmatch.fnmatchcase(d, dso):
                return True
        return False

    def add_order(self, no_cold, wildcards):
        out = []
        for func in self.order:
            order_line = func.order_line()
            if func.has_sect_:
                out.append(order_line)
                continue

            if wildcards:
                out.append(".text*." + order_line)
            elif order_line == "main":
                out.append(".text.main")
                out.append(".text.startup.main")
            else:
                out.append(".text." + order_line)
                out.append(".text.hot." + order_line)
                if not no_cold:
                    out.append(".text.cold." + order_line)
                    out.append(".text.unlikely." + order_line)

        return out
