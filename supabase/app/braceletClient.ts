// src/services/braceletClient.ts
// Reçoit les mesures du bracelet en temps réel depuis Supabase (Realtime).
import { supabase } from './supabase';

export type BraceletData = {
  bpm: number; skin: boolean; removed: boolean; bpmBad: boolean;
  fix: boolean; sat: number; lat: number | null; lon: number | null;
  dist: number; outside: boolean; sos: boolean; rssi: number; at: number; // at = ms
};

export type BraceletEvent =
  | 'OUT_OF_ZONE' | 'BACK_IN_ZONE' | 'BPM_ABNORMAL' | 'BPM_NORMAL'
  | 'REMOVED' | 'WORN_AGAIN' | 'SOS' | 'OFFLINE' | 'ONLINE';

type Handlers = {
  onData: (d: BraceletData) => void;
  onEvent: (e: BraceletEvent, d?: BraceletData) => void; // -> bannière + notif + historique
};

const OFFLINE_AFTER_MS = 45000; // le bracelet publie toutes les 15 s

const map = (r: any): BraceletData => ({
  bpm: r.bpm ?? 0, skin: !!r.skin, removed: !!r.removed, bpmBad: !!r.bpm_bad,
  fix: !!r.fix, sat: r.sat ?? 0, lat: r.lat, lon: r.lon, dist: r.dist ?? 0,
  outside: !!r.outside, sos: !!r.sos, rssi: r.rssi ?? 0,
  at: new Date(r.created_at).getTime(),
});

export async function startBracelet(deviceId: string, h: Handlers) {
  let prev: BraceletData | null = null;
  let last = 0;
  let online = false;

  const handle = (d: BraceletData) => {
    last = d.at;
    if (!online) { online = true; h.onEvent('ONLINE', d); }
    h.onData(d);
    if (prev) {
      if (!prev.outside && d.outside) h.onEvent('OUT_OF_ZONE', d);
      if (prev.outside && !d.outside) h.onEvent('BACK_IN_ZONE', d);
      if (!prev.bpmBad && d.bpmBad) h.onEvent('BPM_ABNORMAL', d);
      if (prev.bpmBad && !d.bpmBad) h.onEvent('BPM_NORMAL', d);
      if (!prev.removed && d.removed) h.onEvent('REMOVED', d);
      if (prev.removed && !d.removed) h.onEvent('WORN_AGAIN', d);
      if (!prev.sos && d.sos) h.onEvent('SOS', d);
    }
    prev = d;
  };

  // 1) dernière mesure connue (affichage immédiat à l'ouverture)
  const { data } = await supabase.from('telemetry').select('*')
    .eq('device_id', deviceId).order('created_at', { ascending: false }).limit(1);
  if (data?.[0]) {
    const d = map(data[0]);
    last = d.at; prev = d; h.onData(d);
    if (Date.now() - d.at < OFFLINE_AFTER_MS) { online = true; }
  }

  // 2) flux temps réel
  const channel = supabase.channel(`telemetry-${deviceId}`)
    .on('postgres_changes',
      { event: 'INSERT', schema: 'public', table: 'telemetry', filter: `device_id=eq.${deviceId}` },
      (p) => handle(map(p.new)))
    .subscribe();

  // 3) détection bracelet hors ligne
  const watchdog = setInterval(() => {
    if (online && last && Date.now() - last > OFFLINE_AFTER_MS) { online = false; h.onEvent('OFFLINE'); }
  }, 5000);

  return () => { clearInterval(watchdog); supabase.removeChannel(channel); };
}
