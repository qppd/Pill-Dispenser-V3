import { NextRequest, NextResponse } from 'next/server';
import { ref, set, get } from 'firebase/database';
import { db } from '@/lib/firebase';

export async function POST(request: NextRequest) {
  try {
    const { userId, status, battery, pillsRemaining, lastDispensed } = await request.json();

    if (!userId) {
      return NextResponse.json({ error: 'User ID required' }, { status: 400 });
    }

    const dispenserRef = ref(db, `dispensers/${userId}`);

    // Get current data
    const snapshot = await get(dispenserRef);
    const currentData = snapshot.val() || {};

    // Update with new data
    const updatedData = {
      ...currentData,
      status: status || currentData.status || 'offline',
      battery: battery !== undefined ? battery : currentData.battery || 100,
      pillsRemaining: pillsRemaining !== undefined ? pillsRemaining : currentData.pillsRemaining || 30,
      lastDispensed: lastDispensed || currentData.lastDispensed || 'Never',
      lastUpdated: new Date().toISOString()
    };

    await set(dispenserRef, updatedData);

    return NextResponse.json({ success: true, data: updatedData });
  } catch (error) {
    console.error('Error updating dispenser:', error);
    return NextResponse.json({ error: 'Internal server error' }, { status: 500 });
  }
}

export async function GET(request: NextRequest) {
  try {
    const { searchParams } = new URL(request.url);
    const userId = searchParams.get('userId');

    if (!userId) {
      return NextResponse.json({ error: 'User ID required' }, { status: 400 });
    }

    const dispenserRef = ref(db, `dispensers/${userId}`);
    const snapshot = await get(dispenserRef);
    const data = snapshot.val();

    return NextResponse.json({ data: data || null });
  } catch (error) {
    console.error('Error fetching dispenser:', error);
    return NextResponse.json({ error: 'Internal server error' }, { status: 500 });
  }
}