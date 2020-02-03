elections::mod_config elections::get_config(){
  config_table _config(get_self(), get_self().value);
  auto conf = _config.get_or_create(get_self(), config() );
  return conf.conf;
}


bool elections::is_candidate(const name& cand){
  candidates_table _candidates(get_self(), get_self().value);
  auto itr = _candidates.find(cand.value);
  if(itr == _candidates.end()){
    return false;
  }
  else{
    return true;
  }
}

void elections::propagate_votes(vector<name> old_votes, vector<name> new_votes, uint64_t old_vote_weight, uint64_t new_vote_weight){

    candidates_table _candidates(get_self(), get_self().value);

    //loop through old votes and substract old weight
    for(name cand : old_votes){
      auto cand_itr = _candidates.find(cand.value);
      if(cand_itr != _candidates.end() ){
        _candidates.modify(cand_itr, same_payer, [&](auto& n) {
            n.total_votes -= old_vote_weight;
        });
      }
    }   
    
    //loop through new votes and add new weight
    for(name cand : new_votes){
      auto cand_itr2 = _candidates.find(cand.value);

      if(cand_itr2 != _candidates.end() ){
        _candidates.modify(cand_itr2, same_payer, [&](auto& n) {
            n.total_votes += new_vote_weight;
        });
      }
    }

}

void elections::sub_stake( const name& account, const extended_asset& value) {
   stake_table _stake( get_self(), account.value);
   const auto& itr = _stake.get( value.quantity.symbol.raw(), "No stake with this symbol.");
   check( itr.balance >= value, "Overdrawn balance");

  if(account != get_self() && itr.balance == value){
    _stake.erase(itr);
    return;
  }
  _stake.modify( itr, same_payer, [&]( auto& a) {
        a.balance -= value;
  });
}


void elections::add_stake( const name& account, const extended_asset& value){
  //stake account must first be opened
  time_point_sec now = time_point_sec(current_time_point());
  stake_table _stake( get_self(), account.value);
  auto itr = _stake.find( value.quantity.symbol.raw() );
  check(itr != _stake.end(), "No stake account open for this asset.");

  _stake.modify( itr, same_payer, [&]( auto& a) {
    a.balance += value;
    a.last_staked = now;
  });
   
}