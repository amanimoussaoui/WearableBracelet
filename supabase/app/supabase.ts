// src/services/supabase.ts
// npx expo install @supabase/supabase-js @react-native-async-storage/async-storage react-native-url-polyfill
import 'react-native-url-polyfill/auto';
import AsyncStorage from '@react-native-async-storage/async-storage';
import { createClient } from '@supabase/supabase-js';

// .env  ->  EXPO_PUBLIC_SUPABASE_URL=https://xxxx.supabase.co
//           EXPO_PUBLIC_SUPABASE_KEY=<clé publishable / anon>
export const supabase = createClient(
  process.env.EXPO_PUBLIC_SUPABASE_URL!,
  process.env.EXPO_PUBLIC_SUPABASE_KEY!,
  { auth: { storage: AsyncStorage, autoRefreshToken: true, persistSession: true, detectSessionInUrl: false } },
);

// --- Compte aidant ---
export const signUp  = (email: string, password: string) => supabase.auth.signUp({ email, password });
export const signIn  = (email: string, password: string) => supabase.auth.signInWithPassword({ email, password });
export const signOut = () => supabase.auth.signOut();

// --- Appairage : rattache le bracelet (device_key) au compte connecté ---
export async function claimDevice(deviceKey: string): Promise<string> {
  const { data, error } = await supabase.rpc('claim_device', { p_key: deviceKey.trim() });
  if (error) throw error;
  return data as string; // device_id
}

// Récupère le bracelet déjà appairé (au relancement de l'app)
export async function getMyDevice() {
  const { data, error } = await supabase.from('devices').select('*').limit(1).maybeSingle();
  if (error) throw error;
  return data;
}
