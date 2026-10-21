-- SafeMind : schéma Supabase (à coller dans SQL Editor puis Run)
create extension if not exists pgcrypto;

create table devices (
  id uuid primary key default gen_random_uuid(),
  device_key text unique not null default encode(gen_random_bytes(16), 'hex'),
  owner uuid references auth.users(id) on delete set null,
  patient_name text default 'Sameh',
  home_lat double precision default 36.8065,
  home_lon double precision default 10.1815,
  radius_out real default 200,
  radius_back real default 180,
  bpm_min int default 45,
  bpm_max int default 120,
  created_at timestamptz default now()
);

create table telemetry (
  id bigint generated always as identity primary key,
  device_id uuid not null references devices(id) on delete cascade,
  created_at timestamptz not null default now(),
  bpm int, skin boolean, removed boolean, bpm_bad boolean,
  fix boolean, sat int, lat double precision, lon double precision,
  dist real, outside boolean, sos boolean, rssi int
);
create index on telemetry (device_id, created_at desc);

alter table devices   enable row level security;
alter table telemetry enable row level security;

create policy "owner lit son bracelet"  on devices   for select using (owner = auth.uid());
create policy "owner modifie son bracelet" on devices for update using (owner = auth.uid());
create policy "owner lit sa télémétrie" on telemetry for select using (
  exists (select 1 from devices d where d.id = device_id and d.owner = auth.uid())
);

-- Le bracelet (rôle anon) n'a PAS accès aux tables : il appelle seulement cette fonction,
-- authentifiée par sa device_key secrète.
create or replace function push_telemetry(
  p_key text, p_bpm int, p_skin boolean, p_removed boolean, p_bpm_bad boolean,
  p_fix boolean, p_sat int, p_lat double precision, p_lon double precision,
  p_dist real, p_outside boolean, p_sos boolean, p_rssi int
) returns void language plpgsql security definer set search_path = public as $$
declare v_id uuid;
begin
  select id into v_id from devices where device_key = p_key;
  if v_id is null then raise exception 'unknown device'; end if;
  insert into telemetry (device_id,bpm,skin,removed,bpm_bad,fix,sat,lat,lon,dist,outside,sos,rssi)
  values (v_id,p_bpm,p_skin,p_removed,p_bpm_bad,p_fix,p_sat,p_lat,p_lon,p_dist,p_outside,p_sos,p_rssi);
  if random() < 0.01 then  -- nettoyage : on garde 7 jours
    delete from telemetry where device_id = v_id and created_at < now() - interval '7 days';
  end if;
end $$;
revoke all on function push_telemetry from public;
grant execute on function push_telemetry to anon;

-- Appairage : l'app envoie la device_key, le bracelet est rattaché au compte connecté.
create or replace function claim_device(p_key text) returns uuid
language plpgsql security definer set search_path = public as $$
declare v_id uuid;
begin
  if auth.uid() is null then raise exception 'not authenticated'; end if;
  update devices set owner = auth.uid() where device_key = p_key and owner is null returning id into v_id;
  if v_id is null then raise exception 'invalid or already claimed'; end if;
  return v_id;
end $$;
revoke all on function claim_device from public;
grant execute on function claim_device to authenticated;

alter publication supabase_realtime add table telemetry;

-- Créer le bracelet puis lire sa clé (à copier dans config.h ET à saisir dans l'app) :
insert into devices (patient_name) values ('Sameh') returning id, device_key;
