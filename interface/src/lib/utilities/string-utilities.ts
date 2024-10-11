export const formatTime = (seconds: number): string => {
	const h = Math.floor(seconds / 3600);
	const m = Math.floor((seconds % 3600) / 60);

	if (h > 0) {
		return m > 0
			? `${h} hour${h > 1 ? 's' : ''} ${m} min${m > 1 ? 's' : ''}`
			: `${h} hour${h > 1 ? 's' : ''}`;
	}
	return m > 0 ? `${m} min${m > 1 ? 's' : ''}` : `< 1 min`;
};

export const utcToHHMM = (utc: number): string => {
	const date = new Date(utc * 1000); // Convert to milliseconds if it's in seconds
	const hours = date.getUTCHours();
	const minutes = date.getUTCMinutes();
	return `${String(hours).padStart(2, '0')}:${String(minutes).padStart(2, '0')}`;
};
