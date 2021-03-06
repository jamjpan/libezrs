CargoWeb::CargoWeb()
    : RSAlgorithm("cargoweb", true), grid_(100) {
  this->batch_time() = 30;
  this->nswapped_ = 0;
}

void CargoWeb::match() {
  this->clear();
  this->prepare();

  for (const Customer& cust : this->customers()) {
    vec_t<ba_cand>& candidates = lookup.at(cust.id());
    bool matched = false;
    auto i = candidates.cbegin();
    while (!matched && i != candidates.cend()) {
      MutableVehicleSptr cand = i->second;
      //print << "cust " << cust.id() << " trying " << cand->id() << std::endl;
      if (!modified.at(cand)) {
        this->to_assign[cand].push_back(cust.id());
        this->modified[cand] = true;
        cand->set_sch(schedules.at(cust.id()).at(cand->id()));
        cand->set_rte(routes.at(cust.id()).at(cand->id()));
        cand->reset_lvn();
        matched = true;
        //print << "  direct accept" << std::endl;
      } else {
        sop_insert(cand, cust, this->retry_sch, this->retry_rte);
        if (chktw(this->retry_sch, this->retry_rte)
         && chkcap(cand->capacity(), this->retry_sch)) {
          this->to_assign[cand].push_back(cust.id());
          this->modified[cand] = true;
          cand->set_sch(this->retry_sch);
          cand->set_rte(this->retry_rte);
          cand->reset_lvn();
          matched = true;
          //print << "  feasible accept" << std::endl;
        } else {  // Replace procedure
          CustId cust_to_remove = randcust(cand->schedule().data());
          //print << "  not feasible; replacing " << cust_to_remove << std::endl;
          if (cust_to_remove != -1) {
            sop_replace(cand, cust_to_remove, cust, this->replace_sch, this->replace_rte);
            if (chktw(this->replace_sch, this->replace_rte)
             && chkcap(cand->capacity(), this->replace_sch)) {
              //print << "  replace accept" << std::endl;
              // Visualize the change
              gui::center(cust.orig());
              gui::vhi(cand->id());
              gui::clinev(cust.id(), cand->id());
              gui::curroute(this->retry_rte);
              gui::newroute(this->replace_rte);
              pause();

              this->to_assign[cand].push_back(cust.id());
              this->modified[cand] = true;
              cand->set_sch(this->replace_sch);
              cand->set_rte(this->replace_rte);
              cand->reset_lvn();
              matched = true;
              auto remove_ptr = std::find(to_assign[cand].begin(), to_assign[cand].end(), cust_to_remove);
              if (remove_ptr != to_assign[cand].end())
                to_assign[cand].erase(remove_ptr);
              else
                to_unassign[cand].push_back(cust_to_remove);
              this->nswapped_++;
            }
          }
        }
      }
      std::advance(i, 1);
    }
  }

  // Batch-commit the solution
  for (const auto& kv : this->modified) {
    if (kv.second == true) {
      this->commit(kv.first);
    }
  }

  gui::reset();
}

void CargoWeb::handle_vehicle(const cargo::Vehicle& vehl) {
  this->grid_.insert(vehl);
}

void CargoWeb::prepare() {
  vec_t<Stop> sch;
  vec_t<Wayp> rte;

  auto add_or_remove = [&](const Customer& cust) {
    if (this->timeout(this->timeout_0)) return true;  // don't continue if no more time
    this->lookup[cust.id()] = {};
    vec_t<MutableVehicleSptr> cands = this->grid_.within(pickup_range(cust), cust.orig());
    for (const MutableVehicleSptr& cand : cands) {
      // Speed heuristic: try only if vehicle's current schedule len < 8 customer stops
      if (cand->schedule().data().size() < 10) {
        this->to_assign[cand] = {};
        this->to_unassign[cand] = {};
        DistInt cost = sop_insert(cand, cust, sch, rte) - cand->route().cost();
        if (chktw(sch, rte) && chkcap(cand->capacity(), sch)) {
          this->lookup.at(cust.id()).push_back(std::make_pair(cost, cand));
          this->schedules[cust.id()][cand->id()] = std::move(sch);
          this->routes[cust.id()][cand->id()] = std::move(rte);
          this->modified[cand] = false;
        }
      }
    }
    if (this->lookup.at(cust.id()).empty()) return true;
    else {
      // Order the candidates in order to access by greedy
      std::sort(this->lookup.at(cust.id()).begin(), this->lookup.at(cust.id()).end(),
        [](const ba_cand& a, const ba_cand& b) { return a.first < b.first; });
      return false;
    }
  };

  this->customers().erase(std::remove_if(this->customers().begin(), this->customers().end(),
    add_or_remove), this->customers().end());
}

void CargoWeb::clear() {
  this->lookup = {};
  this->schedules = {};
  this->routes = {};
  this->modified = {};
  this->to_assign = {};
  this->to_unassign = {};
  this->timeout_0 = hiclock::now();
  this->retry_sch = this->replace_sch = {};
  this->retry_rte = this->replace_rte = {};
}

void CargoWeb::commit(const MutableVehicleSptr& cand) {
  vec_t<CustId>& cadd = this->to_assign.at(cand);
  vec_t<CustId>& cdel = this->to_unassign.at(cand);
  this->assign_or_delay(cadd, cdel, cand->route().data(), cand->schedule().data(), *cand);
  // for (const CustId& cid : cadd) print << "Matched " << cid << " with " << cand->id() << std::endl;
  // for (const CustId& cid : cdel) print << "Removed " << cid << " from " << cand->id() << std::endl;
}

void CargoWeb::end() {
  //print(MessageType::Info) << "swaps: " << this->nswapped_ << std::endl;
  RSAlgorithm::end();
}

void CargoWeb::listen(bool skip_assigned, bool skip_delayed) {
  this->grid_.clear();
  RSAlgorithm::listen(skip_assigned, skip_delayed);
}

