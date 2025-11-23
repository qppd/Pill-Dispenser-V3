import { NextRequest, NextResponse } from 'next/server';
import { ref, get, set } from 'firebase/database';
import { db } from '@/lib/firebase';

export async function POST(request: NextRequest) {
  try {
    const { userId, pills = 1 } = await request.json();

    if (!userId) {
      return NextResponse.json({ error: 'User ID required' }, { status: 400 });
    }

    const dispenserRef = ref(db, `dispensers/${userId}`);
    const snapshot = await get(dispenserRef);
    const currentData = snapshot.val();

    if (!currentData) {
      return NextResponse.json({ error: 'Dispenser not found' }, { status: 404 });
    }

    const updatedData = {
      ...currentData,
      pillsRemaining: Math.max(0, currentData.pillsRemaining - pills),
      lastDispensed: new Date().toLocaleString(),
      lastUpdated: new Date().toISOString()
    };

    await set(dispenserRef, updatedData);

    return NextResponse.json({
      success: true,
      message: `Dispensed ${pills} pill(s)`,
      data: updatedData
    });
  } catch (error) {
    console.error('Error dispensing pills:', error);
    return NextResponse.json({ error: 'Internal server error' }, { status: 500 });
  }
}